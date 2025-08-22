//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "UEIntersectionMergeApp.h"
#include "TrafficAuthorityPacketTypes.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "plexe/PlexeManager.h"
#include "plexe_5g/PlexeInetUtils.h"

namespace plexe {

Define_Module(UEIntersectionMergeApp);

using namespace inet;
using namespace std;
using namespace simu5g;

UEIntersectionMergeApp::UEIntersectionMergeApp() :
        UEBaseApp()
{
    selfStart_ = NULL;
}

UEIntersectionMergeApp::~UEIntersectionMergeApp()
{
    cancelAndDelete(selfStart_);
    cancelAndDelete(takeDistanceMeasurement);
    cancelAndDelete(checkIntersectionExit);
    cancelAndDelete(checkHoldingDistance);
    cancelAndDelete(recordData);
    cancelAndDelete(sendIntersectionUpdate);
}

void UEIntersectionMergeApp::initialize(int stage)
{
    UEBaseApp::initialize(stage);
    // avoid multiple initializations
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    mecAppName = par("mecAppName").stdstringValue();//"MECBaselineIntersectionMergeApp";

    //initializing the auto-scheduling messages
    selfStart_ = new cMessage("selfStart");

    app = FindModule<GeneralPlatooningApp*>::findSubModule(getParentModule());
    ASSERT(app);

    commonEdge = par("commonEdge").stdstringValue();
    distanceThreshold = par("distanceThreshold");
    distanceInterval = SimTime(par("distanceInterval").doubleValue());
    holdingPosition = par("holdingPosition");
    holdingAcceleration = par("holdingAcceleration");

    checkIntersectionExit = new cMessage("checkIntersectionExit");
    checkHoldingDistance = new cMessage("checkHoldingDistance");

    if (positionHelper->isLeader()) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(50 / 3.6);
        takeDistanceMeasurement = new cMessage("takeDistanceMeasurement");
        simtime_t startTime = par("startTime");
        EV << "UEIntersectionMergeApp::initialize - starting in " << startTime << " seconds " << endl;
        scheduleAt(simTime() + startTime, selfStart_);

        // stats logging
        recordData = new cMessage("recordData");
        nodeIdOut.setName("nodeId");
        speedOut.setName("speed");
        posxOut.setName("posx");
        posyOut.setName("posy");
        accelerationOut.setName("acceleration");
        emissionsOut.setName("emissions");
        rounded = SimTime(floor((SimTime().dbl()) * 1000 + 100), SIMTIME_MS);
        scheduleAt(simTime() + rounded, recordData);

        usePeriodicUpdates = par("usePeriodicUpdates");
        if (usePeriodicUpdates) {
            intersectionUpdateInterval = par("intersectionUpdateInterval");
            sendIntersectionUpdate = new cMessage("sendIntersectionUpdate");
        }
    }

}

void UEIntersectionMergeApp::startManeuver()
{
    if (strcmp(commonEdge.c_str(), "") == 0 || !holdingPosition || !holdingAcceleration)
        throw cRuntimeError("<!> Error: impossible to start the maneuver, missing parameters");

    if (positionHelper->isLeader()) {
        state = IntersectionManeuverState::WAIT_RSU;
        sendIntersectionRequest();

        if (usePeriodicUpdates) {
            scheduleAfter(SimTime(intersectionUpdateInterval), sendIntersectionUpdate);
        }
    }
}

IntersectionRequest* UEIntersectionMergeApp::createIntersectionRequest()
{
    int platoonSize = positionHelper->getPlatoonFormation().size();
    double totalLength = platoonSize * traciVehicle->getLength() + (platoonSize - 1) * plexeTraciVehicle->getCACCConstantSpacing();

    IntersectionRequest *req = new IntersectionRequest();
    req->setPlatoonId(positionHelper->getPlatoonId());
    req->setLeaderId(positionHelper->getId());
    req->setDistance(measureRoadDistanceTo(commonEdge, 0));
    req->setSpeed(traciVehicle->getSpeed());
    req->setLength(totalLength);
    req->setVehCount(positionHelper->getPlatoonFormation().size());
    req->setRoadId(traciVehicle->getRoadId().c_str());
    req->setReqTime(simTime().dbl());
    req->setByteLength(100);
    return req;
}

IntersectionUpdate* UEIntersectionMergeApp::createIntersectionUpdate()
{
    int platoonSize = positionHelper->getPlatoonFormation().size();
    double totalLength = platoonSize * traciVehicle->getLength() + (platoonSize - 1) * plexeTraciVehicle->getCACCConstantSpacing();

    IntersectionUpdate *req = new IntersectionUpdate();
    req->setPlatoonId(positionHelper->getPlatoonId());
    req->setLeaderId(positionHelper->getId());
    req->setDistance(measureRoadDistanceTo(commonEdge, 0));
    req->setSpeed(traciVehicle->getSpeed());
    req->setLength(totalLength);
    req->setVehCount(positionHelper->getPlatoonFormation().size());
    req->setRoadId(traciVehicle->getRoadId().c_str());
    req->setReqTime(simTime().dbl());
    req->setByteLength(100);
    return req;
}

void UEIntersectionMergeApp::sendIntersectionRequest()
{
    sendToMECApp(createIntersectionRequest());
}

void UEIntersectionMergeApp::sendIntersectionExitNotification()
{
    IntersectionExitNotification *msg = new IntersectionExitNotification();
    msg->setVehicleId(positionHelper->getId());
    msg->setPlatoonId(positionHelper->getPlatoonId());
    msg->setDestinationId(positionHelper->getLeaderId());
    msg->setExternalId(positionHelper->getExternalId().c_str());
    msg->setByteLength(100);
    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " sending IntersectionExitNotification to " << positionHelper->getLeaderId() << "\n";
    sendCV2XUnicast(msg, positionHelper->getLeaderId());
}

void UEIntersectionMergeApp::handleSelfMsg(cMessage *msg)
{
    if (msg == selfStart_) {
        EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " sending Start MEC App Start request for MEC app " << mecAppName << "\n";
        sendStartMECApp(mecAppName, true, PLATOONING_TRAFFIC_AUTHORITY_APP_ID);
        return;
    }

    if (msg == takeDistanceMeasurement) {
        double distance = measureRoadDistanceTo(commonEdge, 0);
        if (distance < distanceThreshold) {
            startManeuver();
        }
        else {
            scheduleAfter(distanceInterval, takeDistanceMeasurement);
        }
    }

    // leader options
    if (positionHelper->isLeader()) {
        // case #1: checking the distance to the holding point
        if (msg == checkHoldingDistance) {
            // measure the distance
            double d = measureRoadDistanceTo(traciVehicle->getRoadId(), holdingPosition);

            if (d <= evalBrakingPoint()) { // if below the threshold start to slow down
                plexeTraciVehicle->setFixedAcceleration(1, holdingAcceleration);
            }
            else {                     // schedule a new check
                scheduleAfter(SimTime(.1), checkHoldingDistance);
            }
        }

        // case #2: checking the distance to the exit point
        else if (msg == checkIntersectionExit) {
            if (measureRoadDistanceTo(commonEdge, 0) == DBL_MAX) { // exit point crossed
                plexeTraciVehicle->setACCHeadwayTime(.36);
                plexeTraciVehicle->setCruiseControlDesiredSpeed(50 / 3.6);
                plexeTraciVehicle->setFixedAcceleration(0, 0);
            }
            else {
                scheduleAfter(SimTime(.01), checkIntersectionExit);
            }
        }
    }

    // last options
    if (positionHelper->isLast()) {
        // case #1: checking the distance to the exit point
        if (msg == checkIntersectionExit) {
            if (measureRoadDistanceTo(commonEdge, 0) == DBL_MAX) { // exit point crossed
                sendIntersectionExitNotification();
                state = IntersectionManeuverState::END;
            }
            else {
                scheduleAfter(SimTime(.01), checkIntersectionExit);
            }
        }
    }

    if (msg == recordData) {
        VEHICLE_DATA data;
        plexeTraciVehicle->getVehicleData(&data);
        // Write data to output files given in the omnetpp.ini settings
        nodeIdOut.record(positionHelper->getId());
        speedOut.record(data.speed);
        posxOut.record(data.positionX);
        posyOut.record(data.positionY);
        accelerationOut.record(data.acceleration);
        emissionsOut.record(traciVehicle->getCO2Emissions() * 0.1);
        scheduleAt(simTime() + rounded, recordData);
    }

    if (msg == sendIntersectionUpdate) {
        sendToMECApp(createIntersectionUpdate());
        scheduleAfter(SimTime(intersectionUpdateInterval), sendIntersectionUpdate);
    }
}

void UEIntersectionMergeApp::handleMECAppStartAck(inet::Packet *packet)
{
    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " received MEC App Start Ack\n";
    if (positionHelper->isLeader()) {
        scheduleAfter(SimTime(.5), takeDistanceMeasurement);
    }
}

void UEIntersectionMergeApp::handleMECAppMsg(inet::Packet *packet)
{

    if (positionHelper->isLeader()) {
        if (IntersectionClearanceUpdate *clearance = PlexeInetUtils::decapsulate<IntersectionClearanceUpdate>(packet)) {
            if (clearance->getDestinationId() == positionHelper->getId())
                onIntersectionClearanceUpdate(clearance);
            delete clearance;
            return;
        }
        if (IntersectionClearance *clearance = PlexeInetUtils::decapsulate<IntersectionClearance>(packet)) {
            if (clearance->getDestinationId() == positionHelper->getId())
                onIntersectionClearance(clearance);
            delete clearance;
            return;
        }
        if (IntersectionHold *hold = PlexeInetUtils::decapsulate<IntersectionHold>(packet)) {
            if (hold->getDestinationId() == positionHelper->getId())
                onIntersectionHold(hold);
            delete hold;
            return;
        }
        if (IntersectionExitNotification *en = PlexeInetUtils::decapsulate<IntersectionExitNotification>(packet)) {
            if (en->getDestinationId() == positionHelper->getId())
                onIntersectionExitNotification(en);
            delete en;
            return;
        }
    }
    else if (positionHelper->isLast()) {
        if (IntersectionExitNotificationRequest *req = PlexeInetUtils::decapsulate<IntersectionExitNotificationRequest>(packet)) {
            if (req->getDestinationId() == positionHelper->getId())
                onIntersectionExitNotificationRequest(req);
            state = IntersectionManeuverState::WAIT_EXIT;
            delete req;
        }
    }
}

void UEIntersectionMergeApp::onIntersectionClearance(const IntersectionClearance *msg)
{

    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " received IntersectionClearance\n";

    if (state == IntersectionManeuverState::WAIT_RSU) {
        // normal procedure, the platoon is allowed to enter the intersection without waiting
        plexeTraciVehicle->setFixedAcceleration(1, msg->getAcceleration());
        scheduleAfter(SimTime(.5), checkIntersectionExit);
        sendIntersectionExitNotificationRequest();
    }
    else if (state == IntersectionManeuverState::HOLDING) {
        // the platoon is waiting at the holding point -> restart
        plexeTraciVehicle->setFixedAcceleration(0, 0);
        // TODO: speed?
        plexeTraciVehicle->setCruiseControlDesiredSpeed(13.89);
    }
    else
        throw cRuntimeError("<!> Error: unable to process the IntersectionClearance message, invalid state");
}

void UEIntersectionMergeApp::onIntersectionClearanceUpdate(const IntersectionClearanceUpdate* msg)
{
    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " received IntersectionClearanceUpdate\n";

    if (state == IntersectionManeuverState::WAIT_RSU) {
        plexeTraciVehicle->setFixedAcceleration(1, msg->getAcceleration());
    } else if (state == IntersectionManeuverState::HOLDING) {
        throw cRuntimeError("<!> Error: received a ClearanceUpdate while holding. Period updates for the baseline case should be disabled");
    }
    else
        throw cRuntimeError("<!> Error: unable to process the IntersectionClearance message, invalid state");
}

void UEIntersectionMergeApp::onIntersectionHold(const IntersectionHold *msg)
{
    if (state != IntersectionManeuverState::WAIT_RSU)
        throw cRuntimeError("<!> Error: unable to process the IntersectionHold message, invalid state");

    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " received IntersectionHold\n";
    state = IntersectionManeuverState::HOLDING;
    scheduleAfter(SimTime(.5), checkHoldingDistance);
}

void UEIntersectionMergeApp::sendIntersectionExitNotificationRequest()
{
    IntersectionExitNotificationRequest *req = new IntersectionExitNotificationRequest();
    req->setVehicleId(positionHelper->getId());
    req->setPlatoonId(positionHelper->getPlatoonId());
    req->setDestinationId(positionHelper->getPlatoonFormation().at(positionHelper->getPlatoonFormation().size() - 1));
    req->setExternalId(positionHelper->getExternalId().c_str());
    req->setByteLength(100);
    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " sending IntersectionExitNotificationRequest to " << req->getDestinationId() << "\n";
    sendCV2XUnicast(req, req->getDestinationId());
}

void UEIntersectionMergeApp::onIntersectionExitNotification(const IntersectionExitNotification *msg)
{
    sendIntersectionExit();
    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " received IntersectionExitNotification from " << msg->getVehicleId() << "\n";
    state = IntersectionManeuverState::END;
}

void UEIntersectionMergeApp::onIntersectionExitNotificationRequest(const IntersectionExitNotificationRequest *msg)
{
    if (!positionHelper->isLast())
        throw cRuntimeError("<!> Error: only the last vehicle of the platoon is supposed to receive an IntersectionExitNotificationRequest");

    EV << "UEIntersectionMergeApp: " << positionHelper->getId() << " received IntersectionExitNotificationRequest from " << msg->getVehicleId() << "\n";
    // start checking the exit
    scheduleAfter(SimTime(.5), checkIntersectionExit);
    state = IntersectionManeuverState::WAIT_EXIT;
}

IntersectionExit* UEIntersectionMergeApp::createIntersectionExit()
{
    auto e = new IntersectionExit();
    e->setPlatoonId(positionHelper->getPlatoonId());
    e->setLeaderId(positionHelper->getId());
    e->setByteLength(100);
    return e;
}

void UEIntersectionMergeApp::sendIntersectionExit()
{
    sendToMECApp(createIntersectionExit());
}

void UEIntersectionMergeApp::handleCV2XPacket(inet::Packet *packet)
{
    // all the logic for all received messages is in handleMECAppMsg
    // TODO: check that this is not causing problems
    handleMECAppMsg(packet);
}

double UEIntersectionMergeApp::measureRoadDistanceTo(std::string edge, double pos)
{
    std::string myEdge = traciVehicle->getRoadId();
    double myPos = traciVehicle->getLanePosition();
    return traci->getDistanceRoad(myEdge, myPos, edge, pos, true);
}

double UEIntersectionMergeApp::evalBrakingPoint()
{
    return pow(traciVehicle->getSpeed(), 2) / (2 * abs(holdingAcceleration));
}

void UEIntersectionMergeApp::finish()
{

}

} //namespace

