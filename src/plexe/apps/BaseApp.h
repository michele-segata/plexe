//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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

#ifndef BASEAPP_H_
#define BASEAPP_H_

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe/CC_Const.h"
#include "plexe/messages/PlatooningBeacon_m.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"

namespace plexe {

class BaseProtocol;

class BaseApp : public veins::BaseApplLayer {

public:
    virtual void initialize(int stage) override;

protected:
    // id of this vehicle
    int myId;

    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    std::unique_ptr<traci::CommandInterface::Vehicle> plexeTraciVehicle;

    // determines position and role of each vehicle
    BasePositionHelper* positionHelper;

    // lower layer protocol
    BaseProtocol* protocol;

    bool enableLogging;
    double loggingInterval;
    /**
     * Log data about vehicle
     */
    virtual void logVehicleData(bool crashed = false);

    // id of the vehicle
    simsignal_t nodeIdSignal;
    // distance and relative speed
    simsignal_t distanceSignal;
    simsignal_t relativeSpeedSignal;
    // speed and position
    simsignal_t speedSignal;
    simsignal_t posxSignal;
    simsignal_t posySignal;
    // real acceleration and controller acceleration
    simsignal_t accelerationSignal;
    simsignal_t controllerAccelerationSignal;

    // messages for scheduleAt
    cMessage* recordData;
    // message to stop the simulation in case of collision
    cMessage* stopSimulation;

public:
    BaseApp()
    {
        recordData = 0;
        stopSimulation = nullptr;
    }
    virtual ~BaseApp();

    /**
     * Sends a frame
     *
     * @param msg message to be encapsulated into the frame
     * @param destination id of the destination
     */
    void sendFrame(cPacket* msg, int destination);

protected:
    virtual void handleLowerMsg(cMessage* msg) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void handleLowerControl(cMessage* msg) override;

    /**
     * Handles PlatoonBeacons
     */
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb);
};

} // namespace plexe

#endif /* BASEAPP_H_ */
