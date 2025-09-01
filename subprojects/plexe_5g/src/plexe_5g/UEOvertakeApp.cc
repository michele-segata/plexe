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

#include "UEOvertakeApp.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "plexe/PlexeManager.h"
#include "plexe_5g/PlexeInetUtils.h"

namespace plexe {

Define_Module(UEOvertakeApp);

using namespace inet;
using namespace std;
using namespace simu5g;

UEOvertakeApp::UEOvertakeApp() : UEBaseApp()
{
    selfStart_ = NULL;
}

UEOvertakeApp::~UEOvertakeApp()
{
    cancelAndDelete(selfStart_);
    cancelAndDelete(sendUpdateMsg);

}

void UEOvertakeApp::initialize(int stage)
{
    EV << "UEOvertakeApp::initialize - stage " << stage << endl;
    UEBaseApp::initialize(stage);
    // avoid multiple initializations
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    mecAppName = "MECOvertakeApp";
    deviceAppId = par("deviceAppId");

    //initializing the auto-scheduling messages
    selfStart_ = new cMessage("selfStart");

    app = FindModule<GeneralPlatooningApp*>::findSubModule(getParentModule());
    ASSERT(app);

    sendUpdateInterval = par("sendUpdateInterval");
    sendStartOvertakeTime = par("sendStartOvertakeTime");

    overtakeSpeed = par("overtakeSpeed");
    overtakeAcceleration = par("overtakeAcceleration");

    if (positionHelper->isLeader()) {
        sendUpdateMsg = new cMessage("sendUpdateMsg");
        //starting UEOvertakeApp
        simtime_t startTime = par("startTime");
        EV << "UEOvertakeApp::initialize - starting in " << startTime << " seconds " << endl;
        scheduleAt(simTime() + startTime, selfStart_);
    }

    // vehicle id
    nodeIdOut.setName("nodeId");
    // current speed
    speedOut.setName("speed");
    // vehicle position
    posxOut.setName("posx");
    posyOut.setName("posy");
    // vehicle acceleration
    accelerationOut.setName("acceleration");
    // emissions
    emissionsOut.setName("emissions");

    recordData = new cMessage("recordData");
    // init statistics collection. round to 0.1 seconds
    SimTime rounded = SimTime(floor(simTime().dbl() * 1000 + 100), SIMTIME_MS);
    scheduleAt(rounded, recordData);
}

void UEOvertakeApp::handleSelfMsg(cMessage *msg)
{
    if (msg == recordData) {
        VEHICLE_DATA data;
        plexeTraciVehicle->getVehicleData(&data);
        // write data to output files
        nodeIdOut.record(positionHelper->getId());
        accelerationOut.record(data.acceleration);
        speedOut.record(data.speed);
        posxOut.record(data.positionX);
        posyOut.record(data.positionY);
        // CO2 emissions are in mg/s. Given that we log the value 10 times per second, we divide it by 10
        emissionsOut.record(traciVehicle->getCO2Emissions() * 0.1);
        scheduleAfter(SimTime(100, SIMTIME_MS), recordData);
    }
    if (msg == sendUpdateMsg) {
        sendUpdateToMECApp();
        return;
    }

    if (msg == sendStartOvertakeMsg) {
        sendStartOvertakeMessage();
        delete sendStartOvertakeMsg;
        sendStartOvertakeMsg = nullptr;
        return;
    }

    if (msg == selfStart_) {
        sendStartMECApp(mecAppName, true, deviceAppId);
        return;
    }
}

void UEOvertakeApp::handleMECAppStartAck(inet::Packet* packet)
{
    scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);

    if (positionHelper->getPlatoonId() == 1) {
        sendStartOvertakeMsg = new cMessage("sendStartOvertakeMsg");
        scheduleAt(sendStartOvertakeTime, sendStartOvertakeMsg);
    }
}

void UEOvertakeApp::handleMECAppMsg(inet::Packet* packet)
{
    if (const PlatoonSpeedCommand* speedCommand = PlexeInetUtils::decapsulate<PlatoonSpeedCommand>(packet)) {
        onPlatoonSpeedCommand(speedCommand);
    }
    else if (const PlatoonChangeLaneCommand* changeLaneCommand = PlexeInetUtils::decapsulate<PlatoonChangeLaneCommand>(packet)) {
        onPlatoonChangeLaneCommand(changeLaneCommand);
    }
}

void UEOvertakeApp::sendUpdateToMECApp()
{
    PlatoonUpdateMessage *msg = new PlatoonUpdateMessage("PlatoonUpdateMessage");
    populatePlatooningTAQuery(msg);
    veins::Coord pos = mobility->getPositionAt(simTime());
    msg->setX(pos.x);
    msg->setY(pos.y);
    msg->setTime(simTime().dbl());
    msg->setSpeed(mobility->getSpeed());
    msg->setByteLength(PLATOON_UPDATE_SIZE_B);
    sendToMECApp(msg);
    scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " sending update to MEC overtake app\n";
}

void UEOvertakeApp::sendStartOvertakeMessage()
{
    StartOvertakeRequest *msg = new StartOvertakeRequest("StartOvertakeRequest");
    populatePlatooningTAQuery(msg);
    msg->setOvertakeSpeed(overtakeSpeed);
    msg->setOvertakeAcceleration(overtakeAcceleration);
    msg->setByteLength(PLATOON_UPDATE_SIZE_B);
    sendToMECApp(msg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " sending overtake request to MEC overtake app\n";
}

void UEOvertakeApp::populatePlatooningTAQuery(PlatoonTAQuery *msg)
{
    msg->setPlatoonId(positionHelper->getPlatoonId());
    msg->setVehicleId(positionHelper->getId());
    msg->setQueryType(REQUEST);
}

void UEOvertakeApp::onPlatoonSpeedCommand(const PlatoonSpeedCommand *msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " speed command from MEC App: setting speed to " << msg->getSpeed() << "m/s\n";
    plexeTraciVehicle->setCruiseControlDesiredSpeed(msg->getSpeed());
}

void UEOvertakeApp::onPlatoonChangeLaneCommand(const PlatoonChangeLaneCommand *msg)
{
    int changeLane = msg->getChangeLane();
    LOG << "Platoon " << positionHelper->getPlatoonId() << " change command from MEC App: changing lane to the " << (changeLane < 0 ? "right" : "left") << "\n";
    plexeTraciVehicle->changeLaneRelative(changeLane, 0);
}

void UEOvertakeApp::handleCV2XPacket(inet::Packet* packet)
{
    // TODO: decapsulate as needed
    // PlatoonUpdateMessage* frame = PlexeInetUtils::decapsulate<PlatoonUpdateMessage>(container);
}

void UEOvertakeApp::finish()
{
    cancelAndDelete(recordData);
    recordData = nullptr;
}

} //namespace

