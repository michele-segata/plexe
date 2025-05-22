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

#include "UETrafficAuthorityApp.h"
#include "TrafficAuthorityPacketTypes.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "plexe/PlexeManager.h"
#include "plexe_5g/PlexeInetUtils.h"

#include "plexe_5g/messages/PlatoonSearchRequest_m.h"
#include "plexe_5g/messages/PlatoonUpdateMessage_m.h"
#include "plexe_5g/messages/PlatoonApproachRequest_m.h"

namespace plexe {

Define_Module(UETrafficAuthorityApp);

using namespace inet;
using namespace std;
using namespace simu5g;

UETrafficAuthorityApp::UETrafficAuthorityApp() : UEBaseApp()
{
    selfStart_ = NULL;
}

UETrafficAuthorityApp::~UETrafficAuthorityApp()
{
    cancelAndDelete(selfStart_);
    cancelAndDelete(sendUpdateMsg);

}

void UETrafficAuthorityApp::initialize(int stage)
{
    EV << "UETrafficAuthorityApp::initialize - stage " << stage << endl;
    UEBaseApp::initialize(stage);
    // avoid multiple initializations
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    mecAppName = "MECTrafficAuthorityApp";

    //initializing the auto-scheduling messages
    selfStart_ = new cMessage("selfStart");

    app = FindModule<GeneralPlatooningApp*>::findSubModule(getParentModule());
    ASSERT(app);

    sendUpdateInterval = par("sendUpdateInterval");
    sendPlatoonSearchTime = par("sendPlatoonSearchTime");

    if (positionHelper->isLeader()) {
        sendUpdateMsg = new cMessage("sendUpdateMsg");
        //starting UETrafficAuthorityApp
        simtime_t startTime = par("startTime");
        EV << "UETrafficAuthorityApp::initialize - starting in " << startTime << " seconds " << endl;
        scheduleAt(simTime() + startTime, selfStart_);
    }
}

//void UETrafficAuthorityApp::handleHandover(bool b)
//{
//    if (b)
//        EV << positionHelper->getId() << " is starting handover " << b << endl;
//    else
//        EV << positionHelper->getId() << " completed handover " << b << endl;
//}

void UETrafficAuthorityApp::handleSelfMsg(cMessage *msg)
{
    if (msg == sendUpdateMsg) {
        sendUpdateToTA();
        return;
    }

    if (msg == sendPlatoonSearchMsg) {
        sendPlatoonSearch();
        delete sendPlatoonSearchMsg;
        sendPlatoonSearchMsg = nullptr;
        return;
    }

    if (msg == selfStart_) {
        sendStartMECApp(mecAppName, true, PLATOONING_TRAFFIC_AUTHORITY_APP_ID);
        return;
    }
}

void UETrafficAuthorityApp::handleMECAppStartAck(inet::Packet* packet)
{
    scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);

    if (positionHelper->getPlatoonId() == 1) {
        sendPlatoonSearchMsg = new cMessage("sendPlatoonSearchMsg");
        scheduleAt(simTime() + sendPlatoonSearchTime, sendPlatoonSearchMsg);
    }
}

void UETrafficAuthorityApp::handleMECAppMsg(inet::Packet* packet)
{
    if (PlatoonSearchResponse* response = PlexeInetUtils::decapsulate<PlatoonSearchResponse>(packet)) {
        onPlatoonSearchResponse(response);
        delete response;
    }
    else if (PlatoonSpeedCommand* speedCommand = PlexeInetUtils::decapsulate<PlatoonSpeedCommand>(packet)) {
        onPlatoonSpeedCommand(speedCommand);
        delete speedCommand;
    }
    else if (PlatoonContactCommand* contactCommand = PlexeInetUtils::decapsulate<PlatoonContactCommand>(packet)) {
        onPlatoonContactCommand(contactCommand);
        delete contactCommand;
    }
}

void UETrafficAuthorityApp::sendUpdateToTA()
{
    PlatoonUpdateMessage *msg = new PlatoonUpdateMessage();
    populatePlatooningTAQuery(msg);
    veins::Coord pos = mobility->getPositionAt(simTime());
    msg->setX(pos.x);
    msg->setY(pos.y);
    msg->setSpeed(mobility->getSpeed());
    msg->setTime(simTime().dbl());
    msg->setByteLength(PLATOON_UPDATE_SIZE_B);
    sendToMECApp(msg);
    scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " sending update to traffic authority\n";
}

void UETrafficAuthorityApp::sendPlatoonSearch()
{
    PlatoonSearchRequest *msg = new PlatoonSearchRequest("PlatoonSearch");
    populatePlatooningTAQuery(msg);
    PlatoonSearchCriterion criterion = PlatoonSearchCriterion::DISTANCE;
    msg->setSearchCriterion(criterion);
    msg->setByteLength(PLATOON_SEARCH_SIZE_B);
    sendToMECApp(msg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " searching for nearby platoons\n";
}

void UETrafficAuthorityApp::sendPlatoonApproachRequest(int platoonId)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " requesting to approach platoon " << platoonId << "\n";
    PlatoonApproachRequest *msg = new PlatoonApproachRequest();
    populatePlatooningTAQuery(msg);
    msg->setApproachId(platoonId);
    msg->setByteLength(PLATOON_APPROACH_SIZE_B);
    sendToMECApp(msg);
    mySpeed = mobility->getSpeed();
}

void UETrafficAuthorityApp::populatePlatooningTAQuery(PlatoonTAQuery *msg)
{
    msg->setPlatoonId(positionHelper->getPlatoonId());
    msg->setVehicleId(positionHelper->getId());
    msg->setQueryType(REQUEST);
}

void UETrafficAuthorityApp::onPlatoonSearchResponse(PlatoonSearchResponse *msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " got answer from TA: matching platoon ID = " << msg->getMatchingPlatoonId() << "\n";
    sendPlatoonApproachRequest(msg->getMatchingPlatoonId());
}

void UETrafficAuthorityApp::onPlatoonSpeedCommand(PlatoonSpeedCommand *msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " speed command from TA: setting speed to " << msg->getSpeed() << "m/s\n";
    plexeTraciVehicle->setCruiseControlDesiredSpeed(msg->getSpeed());
}

void UETrafficAuthorityApp::onPlatoonContactCommand(PlatoonContactCommand *msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " contact command fromt TA: contacting platoon " << msg->getContactPlatoonId() << " (leader: " << msg->getContactLeaderId() << ")\n";
    plexeTraciVehicle->setCruiseControlDesiredSpeed(mySpeed);
    app->startMergeManeuver(msg->getContactPlatoonId(), msg->getContactLeaderId(), -1);
}

void UETrafficAuthorityApp::handleCV2XPacket(inet::Packet* packet)
{
    // TODO: decapsulate as needed
    // PlatoonUpdateMessage* frame = PlexeInetUtils::decapsulate<PlatoonUpdateMessage>(container);
}

void UETrafficAuthorityApp::finish()
{

}

} //namespace

