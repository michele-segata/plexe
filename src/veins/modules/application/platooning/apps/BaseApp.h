//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef BASEAPP_H_
#define BASEAPP_H_

#include "veins/base/modules/BaseApplLayer.h"

#include "veins/modules/application/platooning/UnicastProtocol.h"
#include "veins/modules/application/platooning/messages/PlatooningBeacon_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/modules/application/platooning/CC_Const.h"

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

class BaseProtocol;

class BaseApp : public Veins::BaseApplLayer {

public:
    virtual void initialize(int stage) override;

protected:
    // id of this vehicle
    int myId;

    Veins::TraCIMobility* mobility;
    Veins::TraCICommandInterface* traci;
    Veins::TraCICommandInterface::Vehicle* traciVehicle;

    // determines position and role of each vehicle
    BasePositionHelper* positionHelper;

    // lower layer protocol
    BaseProtocol* protocol;

    /**
     * Log data about vehicle
     */
    virtual void logVehicleData(bool crashed = false);

    // output vectors for mobility stats
    // id of the vehicle
    cOutVector nodeIdOut;
    // distance and relative speed
    cOutVector distanceOut, relSpeedOut;
    // speed and position
    cOutVector speedOut, posxOut, posyOut;
    // real acceleration and controller acceleration
    cOutVector accelerationOut, controllerAccelerationOut;

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
     * Sends a unicast message
     *
     * @param msg message to be encapsulated into the unicast message
     * @param destination id of the destination
     */
    void sendUnicast(cPacket* msg, int destination);

protected:
    virtual void handleLowerMsg(cMessage* msg) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void handleLowerControl(cMessage* msg) override;

    /**
     * Handles PlatoonBeacons
     */
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb);
};

#endif /* BASEAPP_H_ */
