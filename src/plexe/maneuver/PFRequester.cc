
#include "plexe/maneuver/PFRequester.h"
#include "plexe/apps/PFApp.h"

namespace plexe {

Define_Module(PFRequester);

unsigned int PFRequester::newSessionId(unsigned int maxvalue)
{
    return (unsigned int) uniform(1, maxvalue);
}

void PFRequester::initialize(int stage)
{
    PFModule::initialize(stage);
    if (stage == 2) {
        reqstate = ReqState::SLEEPING;
        timeout = new cMessage("timeoutReq");
        pfcheck = new cMessage("PFcheckReq");
        prepareTimeout = new cMessage("prepareTimeout");
        timeoutDuration = pfapp->par("timeoutDuration").doubleValueInUnit("s");
        if (timeoutDuration < 0.3)
            throw cRuntimeError("timeoutDuration must be greater than 0.3s");
    }
}

void PFRequester::enable(int requestedPlatoonId, int requestedLeaderId)
{
    Enter_Method_Silent();
    PFModule::enable();
    reqstate = ReqState::REQUESTING;
    sessionId = newSessionId();

    dumpOperation(PFTermCode::REQUEST_SENT);
    PFRequest* pr = createPFRequest(requestedPlatoonId, sessionId);

    std::stringstream ss;
    ss << positionHelper->getId() << " --PFRequest--> " << requestedLeaderId <<"\n";
    EV << ss.str();
    pfapp->sendUnicast(pr, requestedLeaderId, PlexeRadioInterfaces::VEINS_11P_SERVICE);
    scheduleAt(simTime() + timeoutDuration, timeout); // Timeout1-REQ
}

void PFRequester::handleMessage(cMessage* msg)
{
    if (msg == pfcheck) {
        if (reqstate == ReqState::MOVINGTOLANE) {
            EV << positionHelper->getId() << "Checking if ready to join... ";
            // Check if we are ready to Join
            int myLane = traciVehicle->getLaneIndex();
            std::pair<std::string, double> res = traciVehicle->getLeader(pfapp->par("maxAdvertiserDistance").doubleValueInUnit("m"));
            int frontVehId = positionHelper->getIdFromExternalId(res.first);
            bool vtp = (res.first.rfind(pfapp->par("vtype").stringValue(), 0) == 0);
            if (myLane == targetLane && frontVehId == targetVeh && vtp) {
                EV << "Ready! Going to READYTOJOIN and notifying targetLeader\n";
                // We are ready on the proper lane and behind targetVeh!
                cancelEvent(prepareTimeout); // Timeout2-REQ
                freezeLane();
                reqstate = ReqState::READYTOJOIN;
                pfapp->sendUnicast(createPFReadyToJoinNotification(frontVehId, sessionId),
                    targetLeader, PlexeRadioInterfaces::VEINS_11P_SERVICE);
                scheduleAt(simTime() + timeoutDuration, timeout); // Timeout3-REQ
            }
            else {
                EV << "NOT Ready!! will retry later\n";
                // still not ready, send a KeepAlive and retry later...
                scheduleAt(simTime() + 0.2, pfcheck);
                pfapp->sendUnicast(createPFKeepAlive(sessionId),
                    targetLeader, PlexeRadioInterfaces::VEINS_11P_SERVICE);
            }
        }
        else if (reqstate == ReqState::MERGING_FOLLOWER) {
            EV << positionHelper->getId() << "Checking if Merge still in progress... ";
            MergeAtBack* mm = dynamic_cast<MergeAtBack*>(pfapp->getMergeManeuver());
            if (pfapp->isInManeuver() || mm->checkDistance->isScheduled()) {
                std::pair<std::string, double> res = traciVehicle->getLeader(pfapp->par("maxAdvertiserDistance").doubleValueInUnit("m"));
                int frontVehId = positionHelper->getIdFromExternalId(res.first);
                bool vtp = (res.first.rfind(pfapp->par("vtype").stringValue(), 0) == 0);
                if (frontVehId != targetVeh || !vtp) {
                    std::stringstream ss;
                    ss << positionHelper->getId() <<
                        "Intruder detected in MergingGap, need to abort! TargetV: "
                        << targetVeh << " but frontV: " << frontVehId << "\n";
                    EV << ss.str();
                    abortRequester(PFTermCode::ABORT_INTRUDERDETECTED, targetLeader);
                }
                else {
                    EV << " it is, check again later\n";
                    scheduleAt(simTime() + 0.2, pfcheck);
                    pfapp->sendUnicast(createPFKeepAlive(sessionId),
                        targetLeader, PlexeRadioInterfaces::VEINS_11P_SERVICE);
                }
            }
            else {
                EV << "MERGE is OVER! Going to WAITCOMPLETEACK and sending PFComplete\n";
                reqstate = ReqState::WAITCOMPLETEACK;
                pfapp->sendUnicast(createPFComplete(sessionId),
                    targetLeader, PlexeRadioInterfaces::VEINS_11P_SERVICE);
                scheduleAt(simTime() + timeoutDuration, timeout); // Timeout4-REQ
            }
        }
    }
    else if (msg == prepareTimeout) {
        // has not been able to get ready for Merge within maxPreparationTime
        EV << "Could not get ready for Merge within maxPreparationTime! Sending PFAbort to "<< targetLeader << "\n";
        abortRequester(PFTermCode::ABORT_TOOLONGLANECHANGE, targetLeader);
    }
    else if (msg == timeout) {
        EV << positionHelper->getId() << " TimeOut expired! sessionId: " << sessionId << "\n";
        if (reqstate == ReqState::REQUESTING || reqstate == ReqState::MOVINGTOLANE || reqstate == ReqState::READYTOJOIN ||
            reqstate == ReqState::MERGING_FOLLOWER || reqstate == ReqState::WAITCOMPLETEACK) {
            abortRequester(PFTermCode::ABORT_REQUESTER_TIMEOUT, targetLeader);
        }
        else
            throw cRuntimeError("Cannot have Requester Timeout while in reqstate=%d", reqstate);
    }
    else {
        throw cRuntimeError("Unkown message for PFRequester!");
    }
}

void PFRequester::handlePFResponse(const PFResponse* msg)
{
    bool error = reqstate != ReqState::REQUESTING || msg->getSessionId() != sessionId;
    if (error) {
        std::stringstream ss;
        if (reqstate != ReqState::REQUESTING)
            ss << "Receiving a PFResponse even if I am not in Requesting state.";
        else if (msg->getSessionId() != sessionId)
            ss << "PFResponse.sessionId=" << msg->getSessionId() << " but mySessionId = " << sessionId << "\n";
        EV << ss.str() << "\n";
        throw cRuntimeError("%s", ss.str().c_str());
    }

    EV << positionHelper->getId() << " got a PFResponse from " << msg->getVehicleId() << ", status=";
    cancelEvent(timeout); // Timeout1-REQ
    if (msg->getStatus() == PFResponseCode::DENY) {
        EV << "DENY: restoringApp...\n";
        dumpOperation(PFTermCode::RESP_DENIED);
        sendPFNotification(PFTermCode::RESP_DENIED); // pfapp->restoreApp(false); // quick restore without graceperiod (i.e. gracePeriod=False)
    }
    else if (msg->getStatus() == PFResponseCode::OK) {
        targetLane = msg->getJoinLane();
        targetVeh = msg->getTargetPlatoonMemberId();
        targetLeader = msg->getVehicleId();

        EV << positionHelper->getId() << " Resp(OK)! Going to MOVINGTOLANE\n";
        EV << " Trying to join with tragetLeader: " << targetLeader << " and targetVeh: " << targetVeh << "\n";
        proactiveLaneChange(targetLane);
        reqstate = ReqState::MOVINGTOLANE;
        scheduleAt(simTime() + 0.1, pfcheck);
        scheduleAt(simTime() + par("maxPreparationTime").doubleValueInUnit("s"), prepareTimeout); // Timeout2-REQ
    }
}

void PFRequester::proactiveLaneChange(int targetLane)
{
    int currentLane = traciVehicle->getLaneIndex();
    std::stringstream ss;
    ss << positionHelper->getId() << " perform proactive lane change!"
        << currentLane << "-->" << targetLane << "\n";
    EV << ss.str();
    plexeTraciVehicle->performPlatoonLaneChange(targetLane);
}

void PFRequester::handlePFJoinAuthorization(const PFJoinAuthorization* msg)
{
    if (reqstate != ReqState::READYTOJOIN)
        throw cRuntimeError("Should not receive PFJoinAuthorization while in state %d", reqstate);
    if (msg->getSessionId() != sessionId || msg->getVehicleId() != targetLeader)
        throw cRuntimeError("This PFJoinAuthorization is wrong!");

    EV << positionHelper->getId() << " got a PFJoinAuthorization... ";
    cancelEvent(timeout); // Timeout3-REQ
    if (msg->getStatus() == PFResponseCode::DENY) {
        EV << "NOT AUTHORIZED :( restoring App\n";
        dumpOperation(PFTermCode::AUTH_DENIED);
        sendPFNotification(PFTermCode::AUTH_DENIED); // pfapp->restoreApp(false);
    }
    else if (msg->getStatus() == PFResponseCode::OK) {
        EV << "AUTHORIZED!! Going to MERGING_FOLLOWER and starting Maneuver\n";
        std::stringstream ss;
        ss << positionHelper->getId() << " is sending MergeRequest to: " << msg->getVehicleId()
            << " for joining Platoon: " << msg->getPlatoonId() << "\n";
        EV << ss.str();
        reqstate = ReqState::MERGING_FOLLOWER;
        pfapp->startMergeManeuver(msg->getPlatoonId(), msg->getVehicleId(), -1);
        scheduleAt(simTime() + 0.2, pfcheck);
    }
}

void PFRequester::handlePFAbort(const PFAbort* msg)
{
    EV << positionHelper->getId() << " received PFAbort: sessionId=" <<
        msg->getSessionId() << " from=" << msg->getVehicleId()
        << " and thus return control to PFApp...\n";
    abortRequester(PFTermCode::ABORTMSGRECEIVED, -101);
}


void PFRequester::handlePFCompleteAck(const PFCompleteAck* msg)
{
    Enter_Method_Silent();
    if (reqstate != ReqState::WAITCOMPLETEACK)
        throw cRuntimeError("Should not receive PFCompleteAck while in state %d", reqstate);
    if (msg->getSessionId() != sessionId || msg->getVehicleId() != targetLeader)
        throw cRuntimeError("This PFCompleteAck is wrong!");

    EV << positionHelper->getId() << " got a PFCompleteAck... ";
    cancelEvent(timeout); // Timeout4-REQ
    dumpOperation(PFTermCode::PF_COMPLETED);
    pfapp->dumpStats(); // we process PFCompleteAck as if they were also PFUpdate, so we dumpStats
    sendPFNotification(PFTermCode::PF_COMPLETED);
    EV << "going to FOLLOWER\n";
}

// void PFRequester::handlePFAbort(const PFAbort* msg)
// {
//    EV << positionHelper->getId() << " Got a PFAbort! (From: " << msg->getVehicleId()
//       << " session: " << msg->getSessionId() << ") Aborting protocol? ...\n";
//
////    if (reqstate == ReqState::ADVERTISING || reqstate == ReqState::LISTENING || reqstate==ReqState::IDLE) {
////        throw cRuntimeError("Discard PFAbort cause I was not currently involved in any Platoform operation");
////    }
//
//    if (sessionId != msg->getSessionId()) {
//        std::stringstream ss;
//        ss << "Ignoring PFAbort cause it is related to session: "<< msg->getSessionId()
//           << " while I am currently involved in cur_sessiond: " << sessionId << "\n";
//        throw cRuntimeError("%s", ss.str().c_str());
//    }
//
//    std::stringstream ss;
//    ss << " Aborting the active-session: " << sessionId << " due to explicit Abort message received from: "
//       << msg->getVehicleId() << "\n";
//    EV << ss.str();
//    pfapp->abortPFApp(sessionId, -101);
//    //abortPFApp(PFTermCode::ABORTMSGRECEIVED, -101);
// }

bool PFRequester::checkAdvertisementFeasible(const PFAdvertisement* msg)
{
    Enter_Method_Silent();
    // Check if it is the case to send a PFRequest
    // 1) advertisers must be "in front of me"
    double advX = msg->getPositionX();
    VEHICLE_DATA data;
    plexeTraciVehicle->getVehicleData(&data);
    // Consider so far an only X-longitudinalScenario!
    double advertiserDistance = advX - data.positionX;

    if (advertiserDistance < pfapp->par("minAdvertiserDistance").doubleValueInUnit("m")) {
        // I should be more than minAdvertiserDistance behind my advertiser
        EV << "Advertisement discarded because advertiser not enough in front of me\n";
        return false;
    }
    else if (advertiserDistance > pfapp->par("maxAdvertiserDistance").doubleValueInUnit("m")) {
        EV << "Advertisement discarded because too far from me\n";
        return false;
    }

    double v2vdist = pfapp->getTargetDistance(pfapp->getTargetController(), msg->getCcDesiredSpeed()) + 4;// 4 is veh Length!!
    double estimatedLastMemberPos = advX - msg->getCurrentPlatoonSize() * v2vdist;

    double distanceFromPlatoTail = estimatedLastMemberPos - data.positionX;
    if (distanceFromPlatoTail < pfapp->par("minAdvertiserDistance").doubleValueInUnit("m")) {
        EV << "Advertisement discarded because platoTail not enough in front of me\n";
        return false;
    }

    // 2) advertisedCruiseSpeed should be in my tolerance range
    double advCcSpeed = msg->getCcDesiredSpeed();
    if (advCcSpeed < pfapp->getMinToleratedSpeed()-EPS || advCcSpeed > pfapp->getMaxToleratedSpeed()+EPS) {
        EV << "Advertisement discarded because advertiser not in my speed-tolerance-range\n";
        return false;
    }

    // 3) check platoonSize limits
    int myPlatoSize = positionHelper->getPlatoonSize();
    if (msg->getCurrentPlatoonSize() + myPlatoSize > pfapp->par("maxPlatoSize").intValue()) {
        EV << "Advertisement discarded because merged platoon size would be too big\n";
        return false;
    }

    // 4) check laneIndexTolerance
    int advertiserLane = msg->getCurrentLane();
    int myLane = traciVehicle->getLaneIndex();
    int laneTolerance = pfapp->par("laneTolerance").intValue();
    if (std::abs(advertiserLane - myLane) > laneTolerance) {
        EV << "PFAdvertisment from " << msg->getVehicleId() << " discarded: laneIndex differ more than laneTolerance\n";
        return false;
    }

    // 5) check laneIndexTolerance
    int mymin = pfapp->getMinToleratedSpeed();
    int mymax = pfapp->getMaxToleratedSpeed();
    int advmin = msg->getMinToleratedSpeed();
    int advmax = msg->getMaxToleratedSpeed();

    int MINV = std::max(mymin, advmin);
    int MAXV = std::min(mymax, advmax);
    if (MAXV < MINV) {
        EV << "PFAdvertisment from " << msg->getVehicleId() << " discarded: tolerated speeds do not overlap at all\n";
        return false;
    }
    int deltaAcceptable = std::abs(MAXV - MINV);
    if (deltaAcceptable < pfapp->par("ccSpeedMinOverlap").doubleValueInUnit("mps")) {
        EV << "PFAdvertisment from " << msg->getVehicleId() << " discarded: tolerated speeds do not overlap enough\n";
        return false;
    }

    bool mnvCompatible = (bool) (msg->getManeuverCapabilitiesCode() & par("maneuverCapabilities").intValue());
    if (!mnvCompatible) {
        EV << "PFAdvertisment from " << msg->getVehicleId() << " discarded: maneuverCapabilities not supported\n";
        return false;
    }

    // All checks passed!
    return true;
}

void PFRequester::abortRequester(plexe::PFTermCode termcode, int abortMsgDestination)
{
    Enter_Method_Silent();
    if ((termcode != PFTermCode::ABORTMSGRECEIVED) && (termcode != PFTermCode::PFREQUEST_TX_FAILURE)
            && (abortMsgDestination != -101))
        pfapp->sendUnicast(createPFAbort(sessionId, termCode2string(termcode)), abortMsgDestination);

    EV << positionHelper->getId() << " requester aborted!\n";
    EV << "restoring ctrl from: " << plexeTraciVehicle->getActiveController()
        << " to ACC: " << ACC << "\n";

    plexeTraciVehicle->setActiveController(ACC);
    plexeTraciVehicle->setACCHeadwayTime(1.2);
    unfreezeLane();

    sendPFNotification(termcode);
    dumpOperation(termcode);

    disable();
}

void PFRequester::dumpOperation(plexe::PFTermCode termcode)
{
    if (!pfapp->isAllowDumpingStats())
        return;

    pfapp->emit(pfapp->pfvehSessId_sig, positionHelper->getId());
    pfapp->emit(pfapp->pfoperation_sig, static_cast<int>(termcode));
    pfapp->emit(pfapp->pfsessionId_sig, sessionId);

    pfapp->emit(pfapp->pfcoordId_sig, targetLeader);

    pfapp->emit(pfapp->pfsessionStartTime_sig, activationTime.dbl());
    pfapp->emit(pfapp->pfsessionStartX_sig, activationPosX);

    if (termcode == PFTermCode::REQUEST_SENT) {
        pfapp->emit(pfapp->pfsessionEndTime_sig, -1.0);
        pfapp->emit(pfapp->pfsessionEndX_sig, -1.0);
        return;
    }
    // for true Termination condition different from REQSENT dump also endPos/endTime
    auto coord = mobility->getPositionAt(simTime());
    pfapp->emit(pfapp->pfsessionEndTime_sig, simTime().dbl());
    pfapp->emit(pfapp->pfsessionEndX_sig, coord.x);
}

void PFRequester::reset()
{
    resetTimers();
    targetLane = -1;
    targetVeh = -1;
    targetLeader = -1;
    activationTime = -1;
}

void PFRequester::resetTimers()
{
    MergeAtBack* mm = dynamic_cast<MergeAtBack*>(pfapp->getMergeManeuver());
    pfapp->cancelEvent(mm->checkDistance);
    cancelEvent(timeout);
    cancelEvent(pfcheck);
    cancelEvent(prepareTimeout);
}

PFReadyToJoinNotification* PFRequester::createPFReadyToJoinNotification(int vehInFrontId, int sessionId)
{
    PFReadyToJoinNotification* msg = new PFReadyToJoinNotification("PFReadyToJoinNotification");
    msg->setVehInFrontId(vehInFrontId);
    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

PFRequest* PFRequester::createPFRequest(int targetPlatoonId, int sessionId)
{
    PFRequest* msg = new PFRequest("PFRequest");
    msg->setTargetPlatoonId(targetPlatoonId);
    msg->setRequesterPlatoonSize(positionHelper->getPlatoonSize());
    msg->setRequesterLane(traciVehicle->getLaneIndex());
    msg->setRequesterCcDesSpeed(plexeTraciVehicle->getCruiseControlDesiredSpeed());
    msg->setMinToleratedSpeed(pfapp->getMinToleratedSpeed());
    msg->setMaxToleratedSpeed(pfapp->getMaxToleratedSpeed());
    msg->setManeuverCapabilitiesCode(par("maneuverCapabilities").intValue());

    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);

    return msg;
}

PFKeepAlive* PFRequester::createPFKeepAlive(int sessionId)
{
    PFKeepAlive* msg = new PFKeepAlive("PFKeepAlive");
    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

PFComplete* PFRequester::createPFComplete(int sessionId)
{
    PFComplete* msg = new PFComplete("PFComplete");
    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

void PFRequester::onPFMessage(PFMessage* pfmsg)
{
    Enter_Method_Silent();
    if (const PFJoinAuthorization* pm = dynamic_cast<const PFJoinAuthorization*>(pfmsg))
        handlePFJoinAuthorization(pm);
    else if (const PFResponse* pm = dynamic_cast<const PFResponse*>(pfmsg))
        handlePFResponse(pm);
    else if (const PFAbort* pm = dynamic_cast<const PFAbort*>(pfmsg))
        handlePFAbort(pm);
    else if (const PFCompleteAck* pm = dynamic_cast<const PFCompleteAck*>(pfmsg))
       handlePFCompleteAck(pm);
    else
        throw cRuntimeError("Unknown format for received PF packet");
}

void PFRequester::disable()
{
    Enter_Method_Silent();
    PFModule::disable();
    reset();
}

void PFRequester::finish()
{
    if (isActive()) {
        //EV << "Dumping end of pending session due to endSimulation\n";
        dumpOperation(PFTermCode::END_SIMULATION);
    }
}

PFRequester::~PFRequester()
{
    cancelAndDelete(pfcheck);
    cancelAndDelete(prepareTimeout);
    cancelAndDelete(timeout);
}

} // namespace plexe
