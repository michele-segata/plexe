
#include "plexe/maneuver/PFAdvertiser.h"
#include "plexe/apps/PFApp.h"

namespace plexe {

Define_Module(PFAdvertiser);

void PFAdvertiser::initialize(int stage)
{
    PFModule::initialize(stage);
    if (stage == 2) {
        advstate = AdvState::SLEEPING;
        timeout = new cMessage("timeoutAdv");
        timeoutDuration = pfapp->par("timeoutDuration").doubleValueInUnit("s");
        if (timeoutDuration < 0.3)
            throw cRuntimeError("timeoutDuration must be greater than 0.3s");
    }
}

void PFAdvertiser::handleMessage(cMessage* msg)
{
    if (msg == timeout) {
        EV << positionHelper->getId() << " TimeOut expired! sessionId: " << sessionId << "\n";
        if (!(advstate == AdvState::JOINWAITING || advstate == AdvState::MERGING_LEADER))
            throw cRuntimeError("Cannot have ADVERTISER Timeout while in advstate=%d", advstate);
        pfapp->sendUnicast(createPFAbort(sessionId, termCode2string(PFTermCode::ABORT_COORD_TIMEOUT)), requesterId);
        sendPFNotification(PFTermCode::ABORT_COORD_TIMEOUT);
        abortAdvertiser();
    }
    else {
        throw cRuntimeError("Unknown event for PFAdvertiser!");
    }
}

void PFAdvertiser::enable(int requesterId, int sessionId)
{
    Enter_Method_Silent();
    PFModule::enable();
    this->requesterId = requesterId;
    this->sessionId = sessionId;
    PFResponse* pfr = createPFResponse(PFResponseCode::OK, sessionId);
    freezeLane();
    pfapp->sendUnicast(pfr, requesterId, PlexeRadioInterfaces::VEINS_11P_SERVICE);
    advstate = AdvState::JOINWAITING;
    scheduleAt(simTime() + timeoutDuration, timeout); // Timeout1-ADV
}

void PFAdvertiser::onPFMessage(PFMessage* pfmsg)
{
    Enter_Method_Silent();
    if (const PFKeepAlive* pm = dynamic_cast<const PFKeepAlive*>(pfmsg))
        handlePFKeepAlive(pm);
    else if (const PFReadyToJoinNotification* pm = dynamic_cast<const PFReadyToJoinNotification*>(pfmsg))
        handlePFReadyToJoinNotification(pm);
    else if (const PFComplete* pm = dynamic_cast<const PFComplete*>(pfmsg))
        handlePFComplete(pm);
    else if (const PFAbort* pm = dynamic_cast<const PFAbort*>(pfmsg))
        handlePFAbort(pm);
    else
        throw cRuntimeError("Unknown format for received PF packet");
}

void PFAdvertiser::reset()
{
    advstate = AdvState::SLEEPING;
    cancelEvent(timeout);
    requesterId = -1;
}

bool PFAdvertiser::checkPFRequestFeasible(const PFRequest* msg)
{
    if (msg->getTargetPlatoonId() != positionHelper->getPlatoonId()) {
        EV << "PFRequest from " << msg->getVehicleId() << " discarded: wrong platoonId!\n";
        throw cRuntimeError("Malformed PFRequest with wrong platoonId sent by %d to %d", msg->getVehicleId(), positionHelper->getId());
    }
    if (msg->getRequesterPlatoonSize() + positionHelper->getPlatoonSize() > pfapp->par("maxPlatoSize").intValue()) {
        EV << "PFRequest from " << msg->getVehicleId() << " discarded: would exceed maxPlatoSize\n";
        return false;
    }
    int reqLane = msg->getRequesterLane();
    int myLane = traciVehicle->getLaneIndex();
    int laneTolerance = pfapp->par("laneTolerance").intValue();
    if (std::abs(reqLane - myLane) > laneTolerance) {
        EV << "PFRequest from " << msg->getVehicleId() << " discarded: laneIndex differ more than laneTolerance\n";
        return false;
    }

    int mymin = pfapp->getMinToleratedSpeed();
    int mymax = pfapp->getMaxToleratedSpeed();
    int reqmin = msg->getMinToleratedSpeed();
    int reqmax = msg->getMaxToleratedSpeed();

    int MINV = std::max(mymin, reqmin);
    int MAXV = std::min(mymax, reqmax);
    if (MAXV < MINV) {
        EV << "PFRequest from " << msg->getVehicleId() << " discarded: tolerated speeds do not overlap at all\n";
        return false;
    }
    int deltaAcceptable = std::abs(MAXV - MINV);
    if (deltaAcceptable < pfapp->par("ccSpeedMinOverlap").doubleValueInUnit("mps")) {
        EV << "PFRequest from " << msg->getVehicleId() << " discarded: tolerated speeds do not overlap enough\n";
        return false;
    }

    double myCCspeed = plexeTraciVehicle->getCruiseControlDesiredSpeed();
    if (myCCspeed > MAXV+EPS || myCCspeed < MINV-EPS) {
        EV << "PFRequest from " << msg->getVehicleId() << " discarded: advertiserCCspeed not in tolerable speed range\n";
        return false;
    }

    bool mnvCompatible = (bool) (msg->getManeuverCapabilitiesCode() & par("maneuverCapabilities").intValue());
    if (!mnvCompatible) {
        EV << "PFRequest from " << msg->getVehicleId() << " discarded: maneuverCapabilities not supported\n";
        return false;
    }

    // all checks passed!
    return true;
}



void PFAdvertiser::handlePFKeepAlive(const PFKeepAlive* msg)
{
    if (!(advstate == AdvState::JOINWAITING || advstate == AdvState::MERGING_LEADER))
        throw cRuntimeError("Should not receive KeepAlive while in state %d", advstate);
    if (msg->getSessionId() != sessionId || msg->getVehicleId() != requesterId)
        throw cRuntimeError("This KeepAlive is wrong! <sessID:%d>?=%d  from:%d?=%d",
                  msg->getSessionId(), sessionId, msg->getVehicleId(), requesterId);

    EV << positionHelper->getId() << " got a PFKeepAlive ==> renewing my timeout\n";
    rescheduleAt(simTime() + timeoutDuration, timeout); // Timeout1-ADV / Timeout2-ADV
}

void PFAdvertiser::handlePFReadyToJoinNotification(const PFReadyToJoinNotification* msg)
{
    if (advstate != AdvState::JOINWAITING)
        throw cRuntimeError("Should not receive PFReadyToJoinNotification while in state %d", advstate);
    if (msg->getSessionId() != sessionId || msg->getVehicleId() != requesterId)
        throw cRuntimeError("This PFReadyToJoinNotification is wrong!");

    int lastMemberId = positionHelper->getLastId();
    if (msg->getVehInFrontId() != lastMemberId)
        throw cRuntimeError("During Preparation the last PlatoonMember has changed?!");

    cancelEvent(timeout); // Timeout1-ADV
    EV << positionHelper->getId() << " got a PFReadyToJoinNotification... ";
    EV << "NULLAOSTA! Going to MERGING_LEADER and sending PFJoinAuthorization\n";
    advstate = AdvState::MERGING_LEADER;

    pfapp->sendUnicast(createPFJoinAuthorization(PFResponseCode::OK, PFManeuverCode::MERGEATBACK, sessionId),
        msg->getVehicleId(), PlexeRadioInterfaces::VEINS_11P_SERVICE);
    scheduleAt(simTime() + timeoutDuration, timeout); // Timeout2-ADV
}

void PFAdvertiser::handlePFComplete(const PFComplete* msg)
{
    if (advstate != AdvState::MERGING_LEADER)
        throw cRuntimeError("Should not receive PFComplete while in state %d", advstate);
    if (msg->getSessionId() != sessionId || msg->getVehicleId() != requesterId)
        throw cRuntimeError("This PFComplete is wrong!");

    EV << positionHelper->getId() << " got a PFComplete... ";
    cancelEvent(timeout); // Timeout2-ADV

    // Updating SUMO Platoon members before unfreezingLane (which enable autolanechange)
    for (int i = 1; i < positionHelper->getPlatoonSize(); i++) {
        std::stringstream ss;
        ss << pfapp->par("vtype").stringValue() << "." << positionHelper->getMemberId(i);
        plexeTraciVehicle->addPlatoonMember(ss.str(), i);
        std::stringstream sst;
        sst << positionHelper->getId() << " adding member = " << ss.str() << "in pos = " << i<<"\n";
        // Send a PFUpdate message to all member for logging purposes
        PFMessage* pfm;
        if (positionHelper->getMemberId(i) == requesterId)
            pfm = createPFCompleteAck(sessionId);
        else
            pfm = createPFUpdate(sessionId);
        pfapp->sendUnicast(pfm, positionHelper->getMemberId(i));
    }

    // notify pfApp that PF_coordinator has completed successfully :)
    sendPFNotification(PFTermCode::COMPLETE_COORD);
}

void PFAdvertiser::handlePFAbort(const PFAbort* msg)
{
    // Advertiser just notifies the PFApp that it got a PFAbort.
    // The PFApp will schedule proper gracePeriod...
    EV << positionHelper->getId() << " received PFAbort: sessionId=" <<
        msg->getSessionId() << " from=" << msg->getVehicleId()
        << " and thus return control to PFApp...\n";
    abortAdvertiser();
    sendPFNotification(PFTermCode::ABORTMSGRECEIVED);
}

void PFAdvertiser::abortAdvertiser()
{
    Enter_Method_Silent();
    EV << positionHelper->getId() << " advertiser aborted!\n";
    EV << "restoring ctrl from: " << plexeTraciVehicle->getActiveController()
        << " to ACC: " << ACC << "\n";
    plexeTraciVehicle->setActiveController(ACC);
    plexeTraciVehicle->setACCHeadwayTime(1.2);
    unfreezeLane();
    disable();
}

PFResponse* PFAdvertiser::createPFResponse(PFResponseCode code, int sessionId)
{
    PFResponse* msg = new PFResponse("PFResponse");
    if (code == PFResponseCode::OK) {
        msg->setStatus(PFResponseCode::OK);
        msg->setJoinLane(traciVehicle->getLaneIndex());
        int lastMemberId = positionHelper->getMemberId(positionHelper->getPlatoonSize() - 1);
        msg->setTargetPlatoonMemberId(lastMemberId);
        msg->setChosenManeuverType(par("maneuverCapabilities").intValue());
    }
    else if (code == PFResponseCode::DENY) {
        msg->setStatus(PFResponseCode::DENY);
        msg->setJoinLane(-1);
        msg->setTargetPlatoonMemberId(-1);
        msg->setChosenManeuverType(-1);
    }

    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

PFJoinAuthorization* PFAdvertiser::createPFJoinAuthorization(int status, int manuverType, int sessionId)
{
    PFJoinAuthorization* msg = new PFJoinAuthorization("PFJoinAuthorization");
    msg->setStatus(status);
    msg->setManuverType(PFManeuverCode::MERGEATBACK);
    msg->setPlatoonId(positionHelper->getPlatoonId());

    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

PFUpdate* PFAdvertiser::createPFUpdate(int sessionId)
{
    PFUpdate* msg = new PFUpdate("PFUpdate");
    msg->setPlatoonId(positionHelper->getPlatoonId());
    msg->setPlatoonLeaderId(positionHelper->getLeaderId());

    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

PFCompleteAck* PFAdvertiser::createPFCompleteAck(int sessionId)
{
    PFCompleteAck* msg = new PFCompleteAck("PFCompleteAck");
    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

void PFAdvertiser::disable()
{
    Enter_Method_Silent();
    PFModule::disable();
    reset();
}

PFAdvertiser::~PFAdvertiser()
{
    cancelAndDelete(timeout);
}

// void PFAdvertiser::handlePFAbort(const PFAbort* msg)
// {
// TODO: abort must be handled by PFApp
//    EV << positionHelper->getId() << " Got a PFAbort! (From: " << msg->getVehicleId()
//       << " session: " << msg->getSessionId() << ") Aborting protocol? ...\n";
//
//    if (advstate != AdvState::MERGING_LEADER || advstate != AdvState::JOINWAITING) {
//        throw cRuntimeError("Discard PFAbort cause I was not currently involved in any Platoform operation");
//    }
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
// abortPFApp(PFOp::ABORTMSGRECEIVED, -101);
// }


} // namespace plexe
