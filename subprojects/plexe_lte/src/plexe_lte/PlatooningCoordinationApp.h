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

#pragma once

#include <string.h>
#include <omnetpp.h>
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"
#include "plexe/apps/GeneralPlatooningApp.h"

#include "plexe_lte/messages/PlatoonTAQuery_m.h"
#include "plexe_lte/messages/PlatoonSearchResponse_m.h"
#include "plexe_lte/messages/PlatoonSpeedCommand_m.h"
#include "plexe_lte/messages/PlatoonContactCommand_m.h"

namespace plexe {

class PlatooningCoordinationApp : public inet::TcpAppBase {

    // invoked when data is received from the traffic authority
    virtual void socketDataArrived(inet::TcpSocket* socket, inet::Packet* msg, bool urgent) override;
    // invoked on self messages
    virtual void handleTimer(cMessage* msg) override;
    // invoked when the connection to the traffic authority has been established
    virtual void socketEstablished(inet::TcpSocket* socket) override;
    // sends a platoon update (platoon id, position) to the traffic authority
    void sendUpdateToTA();
    // sends a query to the traffic authority searching for other platoons
    void sendPlatoonSearch();
    // sends a query to the traffic authority asking help for approaching another platoon
    void sendPlatoonApproachRequest(int platoonId);
    // helper method to populate traffic authority queries packets
    void populatePlatooningTAQuery(PlatoonTAQuery* msg);
    // invoked when a platooning search response have been received
    void onPlatoonSearchResponse(PlatoonSearchResponse* msg);
    // invoked when a change speed command is received
    void onPlatoonSpeedCommand(PlatoonSpeedCommand* msg);
    // invoked when a command to contact a platoon is received from the traffic authority
    void onPlatoonContactCommand(PlatoonContactCommand* msg);

    virtual void handleStartOperation(inet::LifecycleOperation* operation) override {};
    virtual void handleStopOperation(inet::LifecycleOperation* operation) override {};
    virtual void handleCrashOperation(inet::LifecycleOperation* operation) override {};

public:
    ~PlatooningCoordinationApp();
    PlatooningCoordinationApp();

protected:

    // indicates whether we are connected to the traffic authority
    bool connectedToTA;
    // indicates how frequently update messages should be sent to the traffic authority
    simtime_t sendUpdateInterval;
    // indicates when the platoon should send a message for searching for other platoons
    simtime_t sendPlatoonSearchTime;

    // self message for sending updates to the traffic authority
    cMessage* sendUpdateMsg;
    // self message for sending a platoon search query
    cMessage* sendPlatoonSearchMsg;
    // original speed of this platoon
    double mySpeed;

    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    // determines position and role of each vehicle
    BasePositionHelper* positionHelper;
    // traci mobility. used for getting/setting info about the car
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    std::unique_ptr<traci::CommandInterface::Vehicle> plexeTraciVehicle;
    // general platooning app, storing implementation of maneuvers
    GeneralPlatooningApp* app;

    void sendInetPacket(cPacket* packet);
};

} // namespace plexe

