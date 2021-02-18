//
// Copyright (C) 2012-2020 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "PlatoonTrafficAuthority.h"
#include <math.h>

#include "plexe_lte/messages/PlatoonSpeedCommand_m.h"
#include "plexe_lte/messages/PlatoonContactCommand_m.h"
#include "plexe_lte/PlexeInetUtils.h"

#include "plexe/CC_Const.h"

namespace plexe {

using namespace inet;

Define_Module(PlatoonTrafficAuthority);
Define_Module(PlatoonTrafficAuthorityThread);

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_SEARCH_RESPONSE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4 + 1)
#define PLATOON_SPEED_SIZE_B (PLATOON_QUERY_SIZE_B + 4)
#define PLATOON_CONTACT_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

PlatoonTrafficAuthority::PlatoonTrafficAuthority()
{
    traci = nullptr;
}

PlatoonTrafficAuthority::~PlatoonTrafficAuthority()
{
}

void PlatoonTrafficAuthority::initialize(int stage)
{
    TcpServerHostApp::initialize(stage);
    if (stage == INITSTAGE_APPLICATION_LAYER) {
        approachDistanceThreshold = par("approachDistanceThreshold");
        approachSpeedDelta = par("approachSpeedDelta");
    }
}


void PlatoonTrafficAuthority::finish()
{
    TcpServerHostApp::finish();
}

void PlatoonTrafficAuthorityThread::established()
{
    LOG << "connected socket with id=" << sock->getSocketId() << endl;
}

void PlatoonTrafficAuthorityThread::dataArrived(inet::Packet* packet, bool urgent)
{
    cPacket* msg = PlexeInetUtils::decapsulate(packet);

    if (PlatoonUpdateMessage* update = dynamic_cast<PlatoonUpdateMessage*>(msg)) {
        onPlatoonUpdate(update, sock);
    }
    else if (PlatoonSearchRequest* search = dynamic_cast<PlatoonSearchRequest*>(msg)) {
        onPlatoonSearch(search);
    }
    else if (PlatoonApproachRequest* request = dynamic_cast<PlatoonApproachRequest*>(msg)) {
        onPlatoonApproachRequest(request);
    }

    delete packet;
    delete msg;
}

void PlatoonTrafficAuthorityThread::onPlatoonUpdate(const PlatoonUpdateMessage* msg, TcpSocket* socket)
{
    int platoonId = msg->getPlatoonId();
    PlatoonInfo platoonInfo;
    platoonInfo.platoonId = platoonId;
    platoonInfo.platoonLeader = msg->getVehicleId();
    platoonInfo.socket = socket;
    platoonInfo.x = msg->getX();
    platoonInfo.y = msg->getY();
    platoonInfo.speed = msg->getSpeed();
    pta->platoonData[platoonId] = platoonInfo;
    LOG << "Traffic authority got update from platoon " << platoonId << ": position x=" << platoonInfo.x << " y=" << platoonInfo.y << " speed=" << platoonInfo.speed << "\n";

    auto approachingManeuver = pta->approachingManeuvers.find(platoonId);
    if (approachingManeuver != pta->approachingManeuvers.end()) {
        // there is an ongoing approaching maneuver for this platoon, so we need to compute the control action
        computeApproachAction(platoonId, approachingManeuver->second);
    }

}

void PlatoonTrafficAuthorityThread::onPlatoonSearch(const PlatoonSearchRequest* msg)
{
    PlatoonInfo searchingPlatoon, matchingPlatoon;
    double minDistance = 1e9, matchingDistance;
    double minSpeedDifference = 1e9, matchingSpeedDifference;
    bool matchingAhead;

    // search data about the platoon currently searching for another one
    auto thisPlatoon = pta->platoonData.find(msg->getPlatoonId());
    // we don't have data about the platoon that sent the query. stop here
    if (thisPlatoon == pta->platoonData.end()) return;
    searchingPlatoon = thisPlatoon->second;

    //    veins::Coord searchingPosition(searchingPlatoon.x, searchingPlatoon.y);
    double searchingSpeed = searchingPlatoon.speed;

    if (!pta->traci) pta->traci = veins::TraCIScenarioManagerAccess().get()->getCommandInterface();

    for (auto platoon = pta->platoonData.begin(); platoon != pta->platoonData.end(); platoon++) {
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

void PlatoonTrafficAuthorityThread::sendPlatoonSearchResponse(const PlatoonInfo& searchingPlatoon, const PlatoonInfo& matchingPlatoon, double distance, double speedDifference, bool ahead)
{
    LOG << "Sending response back to platoon number " << searchingPlatoon.platoonId << " with candidate platoon " << matchingPlatoon.platoonId << "\n";
    PlatoonSearchResponse* msg = new PlatoonSearchResponse("platoonSearchResponse");
    populateResponse(*msg, searchingPlatoon);
    msg->setMatchingPlatoonId(matchingPlatoon.platoonId);
    msg->setDistance(distance);
    msg->setSpeedDifference(speedDifference);
    msg->setAhead(ahead);
    msg->setByteLength(PLATOON_SEARCH_RESPONSE_SIZE_B);
    sendInetPacket(searchingPlatoon.socket, msg);
}

void PlatoonTrafficAuthorityThread::sendSpeedCommand(const PlatoonInfo& platoon, double speed)
{
    LOG << "Sending speed command to platoon number " << platoon.platoonId << ": set speed to " << speed << "m/s\n";
    PlatoonSpeedCommand* msg = new PlatoonSpeedCommand("platoonSpeedCommand");
    populateResponse(*msg, platoon);
    msg->setSpeed(speed);
    msg->setByteLength(PLATOON_SPEED_SIZE_B);
    sendInetPacket(platoon.socket, msg);
}

void PlatoonTrafficAuthorityThread::sendContactPlatoonCommand(const PlatoonInfo& platoon, int contactPlatoonId, int contactLeaderId)
{
    PlatoonContactCommand* msg = new PlatoonContactCommand("platoonContactCommand");
    populateResponse(*msg, platoon);
    msg->setContactPlatoonId(contactPlatoonId);
    msg->setContactLeaderId(contactLeaderId);
    msg->setByteLength(PLATOON_CONTACT_SIZE_B);
    sendInetPacket(platoon.socket, msg);
}

void PlatoonTrafficAuthorityThread::populateResponse(PlatoonTAQuery& msg, const PlatoonInfo& destination)
{
    msg.setPlatoonId(destination.platoonId);
    msg.setVehicleId(destination.platoonLeader);
}

void PlatoonTrafficAuthorityThread::onPlatoonApproachRequest(const PlatoonApproachRequest* msg)
{
    computeApproachAction(msg->getPlatoonId(), msg->getApproachId());
}

void PlatoonTrafficAuthorityThread::computeApproachAction(int approachingId, int approachedId)
{
    bool ahead;
    auto approaching = pta->platoonData.find(approachingId);
    auto approached = pta->platoonData.find(approachedId);
    // if one of the two platoons doesn't exist, simply ignore the request
    if (approaching == pta->platoonData.end() || approached == pta->platoonData.end()) return;

    double distance = getDistance(approaching->second, approached->second, ahead);
    LOG << "Computing approach action between " << approaching->second.platoonId << " and " << approached->second.platoonId << ": distance=" << distance << "\n";
    if (distance < pta->approachDistanceThreshold) {
        LOG << "Distance smaller than threshold. Telling platoon to switch to autonomous mode\n";
        // if the platoon is close enough to the other, they can communicate and coordinate themself
        sendContactPlatoonCommand(approaching->second, approached->second.platoonId, approached->second.platoonLeader);
        // if there is an ongoing maneuver managed by the TA, remove it after giving autonomous control to platoons
        if (auto approachingManeuver = pta->approachingManeuvers.find(approachingId) != pta->approachingManeuvers.end()) {
            pta->approachingManeuvers.erase(approachingManeuver);
        }
    }
    else {
        double approachSpeed;
        // if the platoon to approach is ahead, the approaching one should speed up
        if (ahead) approachSpeed = approached->second.speed + pta->approachSpeedDelta;
        // otherwise it should slow down
        else approachSpeed = approached->second.speed - pta->approachSpeedDelta;
        LOG << "Distance larger than threshold. Approaching platoon is " << (ahead ? "ahead" : "behind") << ". Sending set speed command (" << approachSpeed << "m/s)\n";
        // send the approach speed to the approaching platoon
        sendSpeedCommand(approaching->second, approachSpeed);
        // keep trace that we are currently sending approach commands to the approaching vehicle
        pta->approachingManeuvers[approaching->second.platoonId] = approached->second.platoonId;
    }
}

double PlatoonTrafficAuthorityThread::getDistance(const PlatoonInfo& first, const PlatoonInfo& second, bool& ahead)
{
    veins::Coord firstPosition(first.x, first.y);
    veins::Coord secondPosition(second.x, second.y);
    // if the vehicles are on the same edge and one is ahead of the other, SUMO cannot compute the route all around the circle
    double distanceFirstToSecond = pta->traci->getDistance(firstPosition, secondPosition, true);
    double distanceSecondToFirst = pta->traci->getDistance(secondPosition, firstPosition, true);
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

void PlatoonTrafficAuthorityThread::sendInetPacket(TcpSocket* socket, cPacket* packet)
{
    inet::Packet* container = PlexeInetUtils::encapsulate(packet, "Plexe_Container");
    socket->send(container);
}

void PlatoonTrafficAuthorityThread::timerExpired(cMessage* timer) {}

} // namespace plexe


