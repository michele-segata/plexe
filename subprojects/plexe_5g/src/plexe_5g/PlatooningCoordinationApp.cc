//
// Copyright (C) 2012-2025 Michele Segata <segata@ccs-labs.org>
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

#include <cmath>
#include "plexe_5g/PlatooningCoordinationApp.h"
#include "plexe_5g/messages/PlatoonSearchRequest_m.h"
#include "plexe_5g/messages/PlatoonUpdateMessage_m.h"
#include "plexe_5g/messages/PlatoonApproachRequest_m.h"

#include "veins/base/utils/FindModule.h"
#include "plexe/PlexeManager.h"

#include "plexe_5g/PlexeInetUtils.h"

#define LOG EV_INFO

using namespace veins;

namespace plexe {

#define round(x) floor((x) + 0.5)

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_UPDATE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4)
#define PLATOON_SEARCH_SIZE_B (PLATOON_QUERY_SIZE_B + 1)
#define PLATOON_APPROACH_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

Define_Module(PlatooningCoordinationApp);

PlatooningCoordinationApp::PlatooningCoordinationApp()
    : connectedToTA(false)
    , sendUpdateMsg(nullptr)
    , sendPlatoonSearchMsg(nullptr)
    , mySpeed(0)
    , positionHelper(nullptr)
    , mobility(nullptr)
    , traci(nullptr)
    , traciVehicle(nullptr)
    , plexeTraci(nullptr)
    , plexeTraciVehicle(nullptr)
    , app(nullptr)
{}

PlatooningCoordinationApp::~PlatooningCoordinationApp()
{
    cancelAndDelete(sendUpdateMsg);
    cancelAndDelete(sendPlatoonSearchMsg);
    sendUpdateMsg = nullptr;
    sendPlatoonSearchMsg = nullptr;
}

void PlatooningCoordinationApp::initialize(int stage)
{

    TcpAppBase::initialize(stage);

    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    connectedToTA = false;
    sendUpdateInterval = par("sendUpdateInterval");
    sendPlatoonSearchTime = par("sendPlatoonSearchTime");

    // get traci interface
    mobility = veins::TraCIMobilityAccess().get(getParentModule());
    ASSERT(mobility);
    traci = mobility->getCommandInterface();
    ASSERT(traci);
    traciVehicle = mobility->getVehicleCommandInterface();
    ASSERT(traciVehicle);
    auto plexe = FindModule<PlexeManager*>::findGlobalModule();
    ASSERT(plexe);
    plexeTraci = plexe->getCommandInterface();
    plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, mobility->getExternalId()));
    positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());
    ASSERT(positionHelper);
    app = FindModule<GeneralPlatooningApp*>::findSubModule(getParentModule());
    ASSERT(app);

    if (positionHelper->isLeader()) {
        sendUpdateMsg = new cMessage("sendUpdateMsg");
        scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);
        if (positionHelper->getPlatoonId() == 1) {
            sendPlatoonSearchMsg = new cMessage("sendPlatoonSearchMsg");
            scheduleAt(simTime() + sendPlatoonSearchTime, sendPlatoonSearchMsg);
        }
    }
}

void PlatooningCoordinationApp::handleTimer(cMessage* msg)
{
    if (msg == sendUpdateMsg) {
        if (!connectedToTA) {
            LOG << "Platoon leader " << positionHelper->getId() << " connecting to traffic authority\n";
            this->connect();
        }
        else {
            sendUpdateToTA();
        }
    }
    else if (msg == sendPlatoonSearchMsg) {
        sendPlatoonSearch();
    }
}

void PlatooningCoordinationApp::socketDataArrived(inet::TcpSocket* socket, inet::Packet* msg, bool urgent)
{
    if (PlatoonSearchResponse* response = PlexeInetUtils::decapsulate<PlatoonSearchResponse>(msg)) {
        onPlatoonSearchResponse(response);
        delete response;
    }
    else if (PlatoonSpeedCommand* speedCommand = PlexeInetUtils::decapsulate<PlatoonSpeedCommand>(msg)) {
        onPlatoonSpeedCommand(speedCommand);
        delete speedCommand;
    }
    else if (PlatoonContactCommand* contactCommand = PlexeInetUtils::decapsulate<PlatoonContactCommand>(msg)) {
        onPlatoonContactCommand(contactCommand);
        delete contactCommand;
    }
    delete msg;
}

void PlatooningCoordinationApp::socketEstablished(inet::TcpSocket* socket)
{
    connectedToTA = true;
    sendUpdateToTA();
}

void PlatooningCoordinationApp::populatePlatooningTAQuery(PlatoonTAQuery* msg)
{
    msg->setPlatoonId(positionHelper->getPlatoonId());
    msg->setVehicleId(positionHelper->getId());
    msg->setQueryType(REQUEST);
}

void PlatooningCoordinationApp::sendInetPacket(cPacket* packet)
{
    inet::Packet* container = PlexeInetUtils::encapsulate(packet, "Plexe_Container");
    sendPacket(container);
}

void PlatooningCoordinationApp::sendUpdateToTA()
{
    PlatoonUpdateMessage* msg = new PlatoonUpdateMessage();
    populatePlatooningTAQuery(msg);
    Coord pos = mobility->getPositionAt(simTime());
    msg->setX(pos.x);
    msg->setY(pos.y);
    msg->setSpeed(mobility->getSpeed());
    msg->setByteLength(PLATOON_UPDATE_SIZE_B);
    sendInetPacket(msg);
    scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " sending update to traffic authority\n";
}

void PlatooningCoordinationApp::sendPlatoonSearch()
{
    PlatoonSearchRequest* msg = new PlatoonSearchRequest("PlatoonSearch");
    populatePlatooningTAQuery(msg);
    PlatoonSearchCriterion criterion = PlatoonSearchCriterion::DISTANCE;
    msg->setSearchCriterion(criterion);
    msg->setByteLength(PLATOON_SEARCH_SIZE_B);
    sendInetPacket(msg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " searching for nearby platoons\n";
}

void PlatooningCoordinationApp::sendPlatoonApproachRequest(int platoonId)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " requesting to approach platoon " << platoonId << "\n";
    PlatoonApproachRequest* msg = new PlatoonApproachRequest();
    populatePlatooningTAQuery(msg);
    msg->setApproachId(platoonId);
    msg->setByteLength(PLATOON_APPROACH_SIZE_B);
    sendInetPacket(msg);
    mySpeed = mobility->getSpeed();
}

void PlatooningCoordinationApp::onPlatoonSearchResponse(PlatoonSearchResponse* msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " got answer from TA: matching platoon ID = " << msg->getMatchingPlatoonId() << "\n";
    sendPlatoonApproachRequest(msg->getMatchingPlatoonId());
}

void PlatooningCoordinationApp::onPlatoonSpeedCommand(PlatoonSpeedCommand* msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " speed command from TA: setting speed to " << msg->getSpeed() << "m/s\n";
    plexeTraciVehicle->setCruiseControlDesiredSpeed(msg->getSpeed());
}

void PlatooningCoordinationApp::onPlatoonContactCommand(PlatoonContactCommand* msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " contact command fromt TA: contacting platoon " << msg->getContactPlatoonId() << " (leader: " << msg->getContactLeaderId() << ")\n";
    plexeTraciVehicle->setCruiseControlDesiredSpeed(mySpeed);
    app->startMergeManeuver(msg->getContactPlatoonId(), msg->getContactLeaderId(), -1);
}

} // namespace plexe
