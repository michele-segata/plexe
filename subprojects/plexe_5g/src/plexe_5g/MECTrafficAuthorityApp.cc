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

#include "MECTrafficAuthorityApp.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"
#include "packets/TrafficAuthorityPacket_m.h"
#include "TrafficAuthorityPacketTypes.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "nodes/mec/utils/httpUtils/httpUtils.h"
#include "nodes/mec/utils/httpUtils/json.hpp"
#include "nodes/mec/MECPlatform/MECServices/packets/HttpResponseMessage/HttpResponseMessage.h"

#include "plexe_5g/PlexeInetUtils.h"
#include "plexe_5g/messages/PlatoonSpeedCommand_m.h"
#include "plexe_5g/messages/PlatoonContactCommand_m.h"

#define LOG EV

namespace plexe {

using namespace inet;
using namespace omnetpp;
using namespace simu5g;

Define_Module(MECTrafficAuthorityApp);

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_SEARCH_RESPONSE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4 + 1)
#define PLATOON_SPEED_SIZE_B (PLATOON_QUERY_SIZE_B + 4)
#define PLATOON_CONTACT_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

MECTrafficAuthorityApp::MECTrafficAuthorityApp(): MECBaseApp()
{
    traci = nullptr;

}
MECTrafficAuthorityApp::~MECTrafficAuthorityApp()
{
}


void MECTrafficAuthorityApp::initialize(int stage)
{
    MECBaseApp::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    approachDistanceThreshold = par("approachDistanceThreshold");
    approachSpeedDelta = par("approachSpeedDelta");

    //testing
    EV << "MECTrafficAuthorityApp::initialize - Mec application "<< getClassName() << " with mecAppId["<< mecAppId << "] has started!" << endl;

}

void MECTrafficAuthorityApp::finish(){
}

void MECTrafficAuthorityApp::handleUEAppMsg(inet::Packet* packet)
{
    // determine its source address/port
    L3Address ueAppAddress = packet->getTag<L3AddressInd>()->getSrcAddress();
    int ueAppPort = packet->getTag<L4PortInd>()->getSrcPort();

    if (PlatoonUpdateMessage* update = PlexeInetUtils::decapsulate<PlatoonUpdateMessage>(packet)) {
        onPlatoonUpdate(update, ueAppAddress, ueAppPort);
        delete update;
    }
    else if (PlatoonSearchRequest* search = PlexeInetUtils::decapsulate<PlatoonSearchRequest>(packet)) {
        onPlatoonSearch(search);
        delete search;
    }
    else if (PlatoonApproachRequest* request = PlexeInetUtils::decapsulate<PlatoonApproachRequest>(packet)) {
        onPlatoonApproachRequest(request);
        delete request;
    }

}

void MECTrafficAuthorityApp::onPlatoonUpdate(const PlatoonUpdateMessage* msg, inet::L3Address ueAppAddress, int ueAppPort)
{
    int platoonId = msg->getPlatoonId();
    PlatoonInfo platoonInfo;
    platoonInfo.platoonId = platoonId;
    platoonInfo.platoonLeader = msg->getVehicleId();
    platoonInfo.leaderAddress = ueAppAddress;
    platoonInfo.leaderPort = ueAppPort;
    platoonInfo.x = msg->getX();
    platoonInfo.y = msg->getY();
    platoonInfo.speed = msg->getSpeed();
    platoonData[platoonId] = platoonInfo;
    LOG << "Traffic authority got update from platoon " << platoonId << ": position x=" << platoonInfo.x << " y=" << platoonInfo.y << " speed=" << platoonInfo.speed << std::endl;

    auto approachingManeuver = approachingManeuvers.find(platoonId);
    if (approachingManeuver != approachingManeuvers.end()) {
        // there is an ongoing approaching maneuver for this platoon, so we need to compute the control action
        computeApproachAction(platoonId, approachingManeuver->second);
    }

}

void MECTrafficAuthorityApp::onPlatoonSearch(const PlatoonSearchRequest* msg)
{
    PlatoonInfo searchingPlatoon, matchingPlatoon;
    double minDistance = 1e9, matchingDistance;
    double minSpeedDifference = 1e9, matchingSpeedDifference;
    bool matchingAhead;

    // search data about the platoon currently searching for another one
    auto thisPlatoon = platoonData.find(msg->getPlatoonId());
    // we don't have data about the platoon that sent the query. stop here
    if (thisPlatoon == platoonData.end()) return;
    searchingPlatoon = thisPlatoon->second;

    //    veins::Coord searchingPosition(searchingPlatoon.x, searchingPlatoon.y);
    double searchingSpeed = searchingPlatoon.speed;

    if (!traci) traci = veins::TraCIScenarioManagerAccess().get()->getCommandInterface();

    for (auto platoon = platoonData.begin(); platoon != platoonData.end(); platoon++) {
        // ignore platoon that is currently searching
        if (platoon->first == msg->getPlatoonId()) continue;

        bool ahead;
        double distance = getDistance(searchingPlatoon, platoon->second, ahead);
        double speedDifference = std::abs(searchingSpeed - platoon->second.speed);

        if (msg->getSearchCriterion() == PlatoonSearchCriterion::DISTANCE) {
            if (distance < minDistance) {
                matchingPlatoon = platoon->second;
                matchingDistance = distance;
                matchingSpeedDifference = speedDifference;
                matchingAhead = ahead;
                minDistance = distance;
            }
        }
        else if (msg->getSearchCriterion() == PlatoonSearchCriterion::SPEED) {
            if (speedDifference < minSpeedDifference) {
                matchingPlatoon = platoon->second;
                matchingDistance = distance;
                matchingSpeedDifference = speedDifference;
                matchingAhead = ahead;
                minSpeedDifference = speedDifference;
            }
        }
    }

    sendPlatoonSearchResponse(searchingPlatoon, matchingPlatoon, matchingDistance, matchingSpeedDifference, matchingAhead);

}

void MECTrafficAuthorityApp::sendPlatoonSearchResponse(const PlatoonInfo& searchingPlatoon, const PlatoonInfo& matchingPlatoon, double distance, double speedDifference, bool ahead)
{
    LOG << "Sending response back to platoon number " << searchingPlatoon.platoonId << " with candidate platoon " << matchingPlatoon.platoonId << std::endl;
    PlatoonSearchResponse* msg = new PlatoonSearchResponse("platoonSearchResponse");
    populateResponse(*msg, searchingPlatoon);
    msg->setMatchingPlatoonId(matchingPlatoon.platoonId);
    msg->setDistance(distance);
    msg->setSpeedDifference(speedDifference);
    msg->setAhead(ahead);
    msg->setByteLength(PLATOON_SEARCH_RESPONSE_SIZE_B);
    sendToUEApp(msg, searchingPlatoon.leaderAddress, searchingPlatoon.leaderPort);
}

void MECTrafficAuthorityApp::sendSpeedCommand(const PlatoonInfo& platoon, double speed)
{
    LOG << "Sending speed command to platoon number " << platoon.platoonId << ": set speed to " << speed << "m/s\n";
    PlatoonSpeedCommand* msg = new PlatoonSpeedCommand("platoonSpeedCommand");
    populateResponse(*msg, platoon);
    msg->setSpeed(speed);
    msg->setByteLength(PLATOON_SPEED_SIZE_B);
    sendToUEApp(msg, platoon.leaderAddress, platoon.leaderPort);
}

void MECTrafficAuthorityApp::sendContactPlatoonCommand(const PlatoonInfo& platoon, int contactPlatoonId, int contactLeaderId)
{
    PlatoonContactCommand* msg = new PlatoonContactCommand("platoonContactCommand");
    populateResponse(*msg, platoon);
    msg->setContactPlatoonId(contactPlatoonId);
    msg->setContactLeaderId(contactLeaderId);
    msg->setByteLength(PLATOON_CONTACT_SIZE_B);
    sendToUEApp(msg, platoon.leaderAddress, platoon.leaderPort);
}

void MECTrafficAuthorityApp::populateResponse(PlatoonTAQuery& msg, const PlatoonInfo& destination)
{
    msg.setPlatoonId(destination.platoonId);
    msg.setVehicleId(destination.platoonLeader);
}

void MECTrafficAuthorityApp::onPlatoonApproachRequest(const PlatoonApproachRequest* msg)
{
    computeApproachAction(msg->getPlatoonId(), msg->getApproachId());
}

void MECTrafficAuthorityApp::computeApproachAction(int approachingId, int approachedId)
{
    bool ahead;
    auto approaching = platoonData.find(approachingId);
    auto approached = platoonData.find(approachedId);
    // if one of the two platoons doesn't exist, simply ignore the request
    if (approaching == platoonData.end() || approached == platoonData.end()) return;

    double distance = getDistance(approaching->second, approached->second, ahead);
    LOG << "Computing approach action between " << approaching->second.platoonId << " and " << approached->second.platoonId << ": distance=" << distance << std::endl;
    if (distance < approachDistanceThreshold) {
        LOG << "Distance smaller than threshold. Telling platoon to switch to autonomous mode\n";
        // if the platoon is close enough to the other, they can communicate and coordinate themself
        sendContactPlatoonCommand(approaching->second, approached->second.platoonId, approached->second.platoonLeader);
        // if there is an ongoing maneuver managed by the TA, remove it after giving autonomous control to platoons
        if (auto approachingManeuver = approachingManeuvers.find(approachingId) != approachingManeuvers.end()) {
            approachingManeuvers.erase(approachingManeuver);
        }
    }
    else {
        double approachSpeed;
        // if the platoon to approach is ahead, the approaching one should speed up
        if (ahead) approachSpeed = approached->second.speed + approachSpeedDelta;
        // otherwise it should slow down
        else approachSpeed = approached->second.speed - approachSpeedDelta;
        LOG << "Distance larger than threshold. Approaching platoon is " << (ahead ? "ahead" : "behind") << ". Sending set speed command (" << approachSpeed << "m/s)" << std::endl;
        // send the approach speed to the approaching platoon
        sendSpeedCommand(approaching->second, approachSpeed);
        // keep trace that we are currently sending approach commands to the approaching vehicle
        approachingManeuvers[approaching->second.platoonId] = approached->second.platoonId;
    }
}

double MECTrafficAuthorityApp::getDistance(const PlatoonInfo& first, const PlatoonInfo& second, bool& ahead)
{
    veins::Coord firstPosition(first.x, first.y);
    veins::Coord secondPosition(second.x, second.y);
    // if the vehicles are on the same edge and one is ahead of the other, SUMO cannot compute the route all around the circle
    double distanceFirstToSecond = traci->getDistance(firstPosition, secondPosition, true);
    double distanceSecondToFirst = traci->getDistance(secondPosition, firstPosition, true);
    double distance;
    if (distanceFirstToSecond < distanceSecondToFirst) {
        distance = distanceFirstToSecond;
        ahead = true;
    }
    else {
        distance = distanceSecondToFirst;
        ahead = false;
    }
    return distance;
}

} //namespace

