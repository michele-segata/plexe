
#include "plexe/apps/PFApp.h"
#include "plexe/traffic/PlatoformTrafficManager.h"

#define VEH_ID_TO_MAC(x) (x + 1)
#define MAC_TO_VEH_ID(x) (x - 1)

// #define EPS (0.01 / 3.6)

using namespace veins;

namespace plexe {

Define_Module(PFApp);

void PFApp::initialize(int stage)
{
    GeneralPlatooningApp::initialize(stage);
    if (stage == 0) {
        platoonId_sig = registerSignal("platoonId_sig");
        platoonSize_sig = registerSignal("platoonSize_sig");
        platoVarX_sig = registerSignal("platoVarX_sig");
        platoleaderFlag_sig = registerSignal("platoleaderFlag_sig");
        pfvehId_sig = registerSignal("pfvehId_sig");

        pfvehSessId_sig = registerSignal("pfvehSessId_sig");
        pfoperation_sig = registerSignal("pfoperation_sig");
        pfsessionId_sig = registerSignal("pfsessionId_sig");
        pfcoordId_sig = registerSignal("pfcoordId_sig");

        pfsessionStartTime_sig = registerSignal("pfsessionStartTime_sig");
        pfsessionEndTime_sig = registerSignal("pfsessionEndTime_sig");
        pfsessionStartX_sig = registerSignal("pfsessionStartX_sig");
        pfsessionEndX_sig = registerSignal("pfsessionEndX_sig");

        gracePeriod = new cMessage("gracePeriod");
        WATCH(sPFstate);
    }
    if (stage == 2) {
        pfprotocol = dynamic_cast<PlatoformProtocol*>(protocol);
        pfreq = check_and_cast<PFRequester*>(this->getModuleByPath("^.pfreq"));
        pfadv = check_and_cast<PFAdvertiser*>(this->getModuleByPath("^.pfadv"));

        maxPlatoSize = par("maxPlatoSize").intValue();
        if (maxPlatoSize < 2)
            throw cRuntimeError("maxPlatoSize must be greater than 1");

        requiredValidAdvertisement = (unsigned int) par("requiredValidAdvertisement").intValue();

        maxAdvertiserDistance = par("maxAdvertiserDistance").doubleValueInUnit("m");
        if (maxAdvertiserDistance < 5.01)
            throw cRuntimeError("maxAdvertiserDistance must be at least 5m");

        minAdvertiserDistance = par("minAdvertiserDistance").doubleValueInUnit("m");
        if (minAdvertiserDistance < 5)
            throw cRuntimeError("minAdvertiserDistance must be at least 5m");

        if (minAdvertiserDistance > minAdvertiserDistance)
            throw cRuntimeError("minAdvertiserDistance must be smaller than maxAdvertiserDistance");

        // subscribe to PF_TYPE messages
        pfprotocol->registerApplication(PF_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"),
            gate("lowerControlIn"), gate("lowerControlOut"));

        // Stay IDLE until start-Advertising/Listening is used by the scenario (in stages > 2)
        setPFstate(PFState::STD_CAM);
    }

    if (stage == 5) {
        double ccDesSpeed = plexeTraciVehicle->getCruiseControlDesiredSpeed();
        double ccDesSpeedTolerance = par("ccDesSpeedTolerance").doubleValueInUnit("mps");

        traffic = dynamic_cast<PlatoformTrafficManager*>(this->getModuleByPath("<root>.traffic"));
        if (traffic) {
            double minSpeed = traffic->par("minSpeed").doubleValueInUnit("mps");
            maxSpeed = traffic->par("maxSpeed").doubleValueInUnit("mps");
            minToleratedSpeed = std::max(ccDesSpeed - ccDesSpeedTolerance, minSpeed);
            maxToleratedSpeed = std::min(ccDesSpeed + ccDesSpeedTolerance, maxSpeed);
            if (positionHelper->isLeader()) {
                if (ccDesSpeed < minSpeed)
                    throw cRuntimeError("ccDesSpeed too low!");
                if (ccDesSpeed > maxSpeed)
                    throw cRuntimeError("ccDesSpeed too high!");
            }
        }

        dumpStats(); // initial dump for every vehicle
        std::string parseqdiag = par("seqdiag").stringValue();
        if (parseqdiag != "") {
            seqdiag = true;
            myfile.open(par("seqdiag").stringValue()+ std::to_string(getParentModule()->getIndex()) + ".puml");
            myfile << "@startuml" << std::endl;
        }
    }
}

void PFApp::startListening()
{
    if (!positionHelper->isLeader())
        throw cRuntimeError("Cannot start Listening cause I am not a leader");

    EV << positionHelper->getId() << " starts listening, seeking for platoform availability\n";
    pfprotocol->subscribeToPFAdvertisment(true);
    setPFstate(PFState::SEEK);
}

void PFApp::stopListening()
{
    if (pfstate != PFState::SEEK)
        throw cRuntimeError("Stopping Listening while not in SEEK state!\n");
    pfprotocol->subscribeToPFAdvertisment(false);
    setPFstate(PFState::STD_CAM);
}

bool PFApp::isActive()
{
    return getActivePFModule() != nullptr;
}

std::string PFApp::pfs2string(PFState pfs)
{
    if (pfs == PFState::STD_CAM)
        return "STD_CAM";
    else if (pfs == PFState::SEEK)
        return "SEEK";
    else if (pfs == PFState::ADV_SEEK)
        return "ADV_SEEK";
    else if (pfs == PFState::REQUESTER)
        return "REQUESTER";
    else if (pfs == PFState::COORD)
        return "COORD";
    else if (pfs == PFState::GRACEP)
        return "GRACEP";
    else
        throw cRuntimeError("Unprintable PFState.");
}

// Advert = Listen+Advert;
void PFApp::startAdvertising()
{
    if (!positionHelper->isLeader())
        throw cRuntimeError("Cannot start advertising cause I am not a leader");
    // Listen...
    startListening();
    // + Advert
    EV << positionHelper->getId() << " starts advertising platoform availability\n";
    pfprotocol->setPlatoonAvailable(true);
    setPFstate(PFState::ADV_SEEK);
}

void PFApp::stopAdvertising()
{
    if (pfstate != PFState::ADV_SEEK)
        throw cRuntimeError("Stopping Advertising while not in advertising state!\n");
    pfprotocol->setPlatoonAvailable(false);
    setPFstate(PFState::SEEK);
}

void PFApp::handleMessage(cMessage* msg)
{
    const PFInternalNotification* inmsg = dynamic_cast<const PFInternalNotification*>(msg);
    if (inmsg) {
        EV << "Got an internal notification from ";
        std::string arrivalGateName = inmsg->getArrivalGate()->getName();
        int noticeCode = inmsg->getNotice();
        delete inmsg;

        setPFstate(PFState::GRACEP);
        double mro = par("gracePeriodMaxRandomOffset").doubleValue();
        double rndOffset = uniform(-mro, mro);
        double waitPeriodShort = par("gracePeriodAfterSuccess").doubleValueInUnit("s") + rndOffset;
        double waitPeriodAbort = par("gracePeriodAfterAbort").doubleValueInUnit("s") + rndOffset;

        if (arrivalGateName == "reqIn") {
            pfreq->disable();
            EV << "pfreq submodule\n";
            if (noticeCode == (int) PFTermCode::PF_COMPLETED) {
                EV << "pfreq completed! We should be simple follower from now on...\n";
                setPFstate(PFState::STD_CAM);
            }
            else if (noticeCode == (int) PFTermCode::RESP_DENIED || noticeCode == (int) PFTermCode::AUTH_DENIED) {
                EV << "pfreq RESP-DENIED! Graceperiod for " << waitPeriodShort << " sec...\n";
                setPFstate(PFState::GRACEP);
                scheduleAt(simTime() + waitPeriodShort, gracePeriod);
            }
            else {
                // all other reasons notified by pfreq should be abort reasons...
                EV << positionHelper->getId() << "aborted, "
                    "so going to Graceperiod for " << waitPeriodAbort << " sec...\n";
                setPFstate(PFState::GRACEP);
                scheduleAt(simTime() + waitPeriodAbort, gracePeriod);
            }
        }
        else if (arrivalGateName == "advIn") {
            EV << "pfadv submodule\n";
            pfadv->disable();
            if (noticeCode == (int) PFTermCode::COMPLETE_COORD) {
                EV << "pfadv completed! Graceperiod for " << waitPeriodShort << " sec...\n";
                dumpStats(); // only coord dump stats here, followers do it while handling PFUpdate
                setPFstate(PFState::GRACEP);
                scheduleAt(simTime() + waitPeriodShort, gracePeriod);
            }
            else if (noticeCode == (int) PFTermCode::ABORT_COORD_TIMEOUT ||
                noticeCode == (int) PFTermCode::ABORTMSGRECEIVED) {
                EV << "pfadv aborted! Graceperiod for " << waitPeriodAbort << " sec...\n";
                setPFstate(PFState::GRACEP);
                scheduleAt(simTime() + waitPeriodAbort, gracePeriod);
            }
        }
        //TODO:check more... whatever the termination condition
        // let's stop pending maneuvers
        joinManeuver->abortManeuver();
        mergeManeuver->abortManeuver();
        setInManeuver(false, nullptr);
    }
    else {
        if (msg->isSelfMessage())
            PFApp::handleSelfMsg(msg);
        else
            PFApp::handleLowerMsg(msg);
    }
}

void PFApp::handleSelfMsg(cMessage* msg)
{
    if (msg == gracePeriod) {
        EV << positionHelper->getId() << " Grace Period is over...\n";
        if (positionHelper->isLeader()) {
            EV << positionHelper->getId() << " Leader is Unfreezing Lane after GP\n";
            plexeTraciVehicle->enableAutoLaneChanging(true);
        }
        restoreBackupPoint();
        EV << positionHelper->getId() << " Choosing what to do after Grace Period...\n";
        if (preGPstate == PFState::ADV_SEEK) {
            EV << " I was an ADV_SEEK... ";
            if (keepAdvertising()) {
                EV << "I keep advertising :)\n";
                startAdvertising();
            }
            else {
                EV << "but cannot keep advertising\n";
                setPFstate(PFState::STD_CAM);
            }
        }
        else if (preGPstate == PFState::SEEK) {
            EV << " I was a SEEKER... ";
            if (keepListening()) {
                EV << " I keep Listening :) \n";
                startListening();
            } else {
                EV << "but cannot keep listening\n";
                setPFstate(PFState::STD_CAM);
            }
        }
        else {
            throw cRuntimeError("Unknown previous state when GracePeriod expired\n");
        }
        preGPstate = PFState::STD_CAM;
    }
    else
        GeneralPlatooningApp::handleSelfMsg(msg);
}

void PFApp::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);

    cPacket* enc = frame->getEncapsulatedPacket();
    ASSERT2(enc, "received a BaseFrame1609_4s with nothing inside");

    if (enc->getKind() == PF_TYPE) {
        // EV << "got a PLATOFORM packet\n";
        PFMessage* pfmsg = check_and_cast<PFMessage*>(frame->decapsulate());

        if (const PFRequest* pm = dynamic_cast<const PFRequest*>(pfmsg))
            handlePFRequest(pm);
        else if (const PFUpdate* pm = dynamic_cast<const PFUpdate*>(pfmsg)) {
            PFApp::handlePFUpdate(pm); // SIMPLE-FOLLOWER
        }
        else {
            PFModule* pfmod = getActivePFModule();
            if (pfmod == nullptr) {
                std::stringstream ss;
                ss << pfmsg->getClassAndFullName() << " PF packet could not be handled cause no module is active!";
                throw cRuntimeError("%s", ss.str().c_str());
            }
            else {
                // delegate message to active PF module
                pfmod->onPFMessage(pfmsg);
            }
        }
        delete msg;
        delete pfmsg;
        return;

    }
    // NON-PF messages (e.g., Maneuver messages) should be handled by parent-class
    GeneralPlatooningApp::handleLowerMsg(msg);
}

void PFApp::handlePFAdvertisement(const PFAdvertisement* msg)
{
    EV << positionHelper->getId() << " received PFAdvertisement from: " << msg->getVehicleId() << "\n";
    if (!(pfstate == PFState::ADV_SEEK || pfstate == PFState::SEEK)) {
        EV << " Ignoring PFAdvertisement cause I am not ADV_SEEK nor SEEKing for platoons\n";
        return;
    }
    if (gracePeriod->isScheduled() || pfstate == PFState::GRACEP) {
        EV << " Ignoring PFAdvertisement cause I am currently in a grace-period\n";
        return;
    }
    if (msg->getVehicleId() == positionHelper->getLeaderId()) {
        EV << " Ignoring PFAdvertisement of my own PlatoonLeader\n";
        return;
    }
    // If not useful (unfeasible advert) then avoid processing/storing...
    if (!pfreq->checkAdvertisementFeasible(msg))
        return;

    // but if it is instead feasible...
    eCamHistory[msg->getVehicleId()].push_back(msg->getTime());
    EV << positionHelper->getId() << " pushing " << msg->getVehicleId()
        << " in my history that now has size "<< eCamHistory[msg->getVehicleId()].size() << "\n";
    if (eCamHistory[msg->getVehicleId()].size() < requiredValidAdvertisement)
        return;
    if (eCamHistory[msg->getVehicleId()].size() >= requiredValidAdvertisement) {
        // send request and go to REQUESTING state... also clear advertisments-history
        eCamHistory.clear();

        createBackupPoint();
        preGPstate = pfstate;

        if (pfstate == PFState::ADV_SEEK)
            stopAdvertising();
        stopListening();

        pfstate= PFState::REQUESTER;

        pfreq->enable(msg->getPlatoonId(), msg->getVehicleId());
    }
}

void PFApp::handlePFRequest(const PFRequest* msg)
{
    EV << positionHelper->getId() << " got a PFRequest from " << msg->getVehicleId() << "... ";
    if (pfreq->isActive() || pfadv->isActive() || pfstate != PFState::ADV_SEEK) {
        EV << "PFRequest discarded cause busy already! (->DENY to )" << msg->getVehicleId() << "\n";
        sendUnicast(pfadv->createPFResponse(PFResponseCode::DENY, msg->getSessionId()),
            msg->getVehicleId(), PlexeRadioInterfaces::VEINS_11P_SERVICE);
        return;
    }
    if (gracePeriod->isScheduled()) {
        EV << " discarded cause I am in a Grace Period! (->DENY to )" << msg->getVehicleId() << "\n";
        sendUnicast(pfadv->createPFResponse(PFResponseCode::DENY, msg->getSessionId()),
            msg->getVehicleId(), PlexeRadioInterfaces::VEINS_11P_SERVICE);
        return;
    }

    if (!pfadv->checkPFRequestFeasible(msg)) {
        EV << "Discarded cause not feasible!\n";
        sendUnicast(pfadv->createPFResponse(PFResponseCode::DENY, msg->getSessionId()),
            msg->getVehicleId(), PlexeRadioInterfaces::VEINS_11P_SERVICE);
        return;
    }
    else {
        EV << "Accepted! Sending back PFResponse to " << msg->getVehicleId() << "\n";
        // preGPState must be invoked before stopping advert+listen for correct state recording...
        preGPstate = pfstate;
        stopAdvertising();
        stopListening();
        createBackupPoint();

        setPFstate(PFState::COORD);
        pfadv->enable(msg->getVehicleId(), msg->getSessionId());
    }
}

void PFApp::handlePFUpdate(const PFUpdate* msg)
{
    if (msg->getPlatoonId() != positionHelper->getPlatoonId())
        throw cRuntimeError("Wrong platoonId in PFUPDATE msg");
    if (msg->getPlatoonLeaderId() != positionHelper->getLeaderId())
        throw cRuntimeError("Wrong platoonLeaderId in PFUPDATE msg");
    EV << positionHelper->getId() << " got a PFUpdate\n";
    dumpStats();
}

void PFApp::receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details)
{
    Enter_Method_Silent();
    if (id == Mac1609_4::sigRetriesExceeded) {
        BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(value);
        PFMessage* msg = dynamic_cast<PFMessage*>(frame->getEncapsulatedPacket());
        if (msg) {
            if (dynamic_cast<PFAbort*>(msg))
                return; // ignore TX Failure for Abort Messages

            if (dynamic_cast<PFComplete*>(msg))
                throw cRuntimeError("The handling of TX-Failure of PFComplete is not implemented yet.");
            if (dynamic_cast<PFUpdate*>(msg))
                throw cRuntimeError("The handling of TX-Failure of PFUpdate is not implemented yet.");

            if (pfstate == PFState::SEEK || pfstate == PFState::ADV_SEEK)
                throw cRuntimeError("Should not handle unicast tx-failure while in state %d", pfstate);

            int destId = MAC_TO_VEH_ID(frame->getRecipientAddress());
            std::stringstream ss;
            ss << "Unicast PF packet TX failed! From: " << positionHelper->getId()
                << " To: " << destId << "\n";

            EV << ss.str();

            auto am = getActivePFModule();
            if (am == nullptr)
                throw cRuntimeError("unicast TX failure while not having an active requester nor advertiser module");
            if (dynamic_cast<PFRequester*>(am)) {
                if (dynamic_cast<PFRequest*>(msg))
                    pfreq->abortRequester(PFTermCode::PFREQUEST_TX_FAILURE, destId);
                else
                    pfreq->abortRequester(PFTermCode::TX_FAILURE, destId);
            }
            if (dynamic_cast<PFAdvertiser*>(am))
                pfadv->abortAdvertiser();

            return; // Dont pass it to GeneralPlatooninApp
        }

        ManeuverMessage* mmsg = dynamic_cast<ManeuverMessage*>(frame->getEncapsulatedPacket());
        if (mmsg) {
            int destId = MAC_TO_VEH_ID(frame->getRecipientAddress());
            std::stringstream ss;
            ss << "Unicast MANEUVER packet TX failed! From: " << positionHelper->getId()
                << " To: " << destId << "\n";
            getSimulation()->getActiveEnvir()->alert(ss.str().c_str());
            throw cRuntimeError("The handling of TX-Failure of ManeuverMessages is not implemented yet.");
            // NB: we would need a mechanism to abort mergemaneuver in general, reverting merging-leader
            // that activate CACC to their previous controller and separating oldPlatoon members from
            // new ones...
        }
    }
    GeneralPlatooningApp::receiveSignal(src, id, value, details);
}

PFModule* PFApp::getActivePFModule()
{
    if (pfreq->isActive() && !pfadv->isActive())
        return pfreq;
    else if (!pfreq->isActive() && pfadv->isActive())
        return pfadv;
    else if (pfreq->isActive() && pfadv->isActive())
        throw cRuntimeError("Both PFModules are active!!");
    else
        return nullptr;
}

void PFApp::onPlatoonBeacon(const PlatooningBeacon* pb)
{
    if (PFAdvertisement* msg = dynamic_cast<PFAdvertisement*>(const_cast<PlatooningBeacon*>(pb))) {
        if (pfprotocol->isEcamSubscribed())
            handlePFAdvertisement(msg);
    }
    GeneralPlatooningApp::onPlatoonBeacon(pb);
}

void PFApp::dumpStats(std::string optionalMessage)
{
    if (!allowDumpingStats)
        return;

    auto coord = mobility->getPositionAt(simTime());
    emit(platoVarX_sig, coord.x);
    emit(pfvehId_sig, myId);
    emit(platoleaderFlag_sig, positionHelper->isLeader());

    if (optionalMessage == "") {
        emit(platoonId_sig, positionHelper->getPlatoonId());
        emit(platoonSize_sig, positionHelper->getPlatoonSize());
    } else if (optionalMessage == COOLDOWNBARRIER) {
        emit(platoonId_sig, BARRIER_VALUE);
        emit(platoonSize_sig, BARRIER_VALUE);
    }
    else if (optionalMessage == SWITCHOFFBARRIER) {
        emit(platoonId_sig, BARRIER_VALUE);
        emit(platoonSize_sig, BARRIER_VALUE);
    }

}

// std::string PFApp::pfstateString(enum PFState pfstate)
// {
//    std::string retval;
//    switch (pfstate) {
//    case PFState::IDLE:
//        retval = "IDLE";
//        break;
//    case PFState::ADVERTISING:
//        retval = "ADVERTISING";
//        break;
//    case PFState::JOINWAITING:
//        retval = "JOINWAITING";
//        break;
//    case PFState::MERGING_LEADER:
//        retval = "MERGING_LEADER";
//        break;
//    case PFState::LISTENING:
//        retval = "LISTENING";
//        break;
//    case PFState::REQUESTING:
//        retval = "REQUESTING";
//        break;
//    case PFState::MOVINGTOLANE:
//        retval = "MOVINGTOLANE";
//        break;
//    case PFState::READYTOJOIN:
//        retval = "READYTOJOIN";
//        break;
//    case PFState::MERGING_FOLLOWER:
//        retval = "MERGING_FOLLOWER";
//        break;
//    case PFState::WAITCOMPLETEACK:
//        retval = "WAITCOMPLETEACK";
//        break;
//    case PFState::FOLLOWER:
//        retval = "FOLLOWER";
//        break;
//    default:
//        throw cRuntimeError("Unknown pftstate %d.", pfstate);
//    }
//    return retval;
// }

void PFApp::createBackupPoint()
{
    EV << "Creating Backup Point for node " << positionHelper->getId() << "\n";
    backupPoint.pfstate = pfstate;
    backupPoint.role = getPlatoonRole();
}

void PFApp::restoreBackupPoint()
{
    EV << "Restoring Backup Point of node " << positionHelper->getId() << "\n";
    // EV << "Restoring pfstate: " << pfstateString(pfstate) << "-->" << pfstateString(backupPoint.pfstate) << "\n";
    // Restoring pfstate and platoRole
    setPFstate(backupPoint.pfstate);
    setPlatoonRole(backupPoint.role);

    // clearing backup to avoid inconsistencies
    backupPoint.pfstate = PFState::STD_CAM;
    backupPoint.role = PlatoonRole::NONE;
}

void PFApp::sendUnicast(cPacket* msg, int destination, enum PlexeRadioInterfaces interface)
{
    Enter_Method_Silent();
    take(msg);
    BaseFrame1609_4* frame = new BaseFrame1609_4("BaseFrame1609_4", msg->getKind());
    frame->setRecipientAddress(destination);
    frame->setChannelNumber(static_cast<int>(Channel::cch));
    frame->setUserPriority(4); // TODO get priority configured for the used NIC/MAClayer
    frame->encapsulate(msg);
    PlexeInterfaceControlInfo* ctrl = new PlexeInterfaceControlInfo();
    ctrl->setInterfaces(interface);
    frame->setControlInfo(ctrl);
    if (seqdiag)
        myfile << myId << " --> " << destination << ": \"" << simTime() <<
        " " << msg->getFullName() << "\"" << std::endl;
    sendDown(frame);
}

void PFApp::sendUnicast(cPacket* msg, int destination)
{
    Enter_Method_Silent();
    take(msg);
    PFApp::sendUnicast(msg, destination, PlexeRadioInterfaces::VEINS_11P_SERVICE);
}

PFApp::~PFApp()
{
    MergeAtBack* mm = dynamic_cast<MergeAtBack*>(mergeManeuver);
    cancelEvent(mm->checkDistance);
    cancelAndDelete(gracePeriod);
}

void PFApp::finish()
{
    if (seqdiag) {
        myfile << "@enduml" << std::endl;
        myfile.close();
    }
}

bool PFApp::keepAdvertising()
{
    bool reachedMaxPlatoSize = positionHelper->getPlatoonSize() >= maxPlatoSize;
    return !cooldown_passed && !switchoff_passed && !reachedMaxPlatoSize;
}

bool PFApp::keepListening()
{
    bool reachedMaxPlatoSize = positionHelper->getPlatoonSize() >= maxPlatoSize;
    return !cooldown_passed && !switchoff_passed && !reachedMaxPlatoSize;
}

} // namespace plexe
