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

#include "plexe/apps/SimplePlatooningApp.h"
#include "plexe/protocols/BaseProtocol.h"

namespace plexe {

Define_Module(SimplePlatooningApp);

void SimplePlatooningApp::initialize(int stage)
{

    BaseApp::initialize(stage);

    if (stage == 1) {
        // connect application to protocol
        protocol->registerApplication(BaseProtocol::BEACON_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
        enableLogging();
    }
}

void SimplePlatooningApp::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);

    cPacket* enc = frame->decapsulate();
    ASSERT2(enc, "received a BaseFrame1609_4 with nothing inside");

    if (enc->getKind() == BaseProtocol::BEACON_TYPE) {
        onPlatoonBeacon(check_and_cast<PlatooningBeacon*>(enc));
    }
    else {
        error("received unknown message type");
    }

    delete frame;
}

void SimplePlatooningApp::onPlatoonBeacon(const PlatooningBeacon* pb)
{
    if (positionHelper->isInSamePlatoon(pb->getVehicleId())) {
        // if the message comes from the leader
        if (pb->getVehicleId() == positionHelper->getLeaderId()) {
            plexeTraciVehicle->setLeaderVehicleData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed(), pb->getPositionX(), pb->getPositionY(), pb->getTime());
        }
        // if the message comes from the vehicle in front
        if (pb->getVehicleId() == positionHelper->getFrontId()) {
            plexeTraciVehicle->setFrontVehicleData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed(), pb->getPositionX(), pb->getPositionY(), pb->getTime());
        }
        // send data about every vehicle to the CACC controllers
        // controllers will then pick the data of vehicles they are interested in
        struct VEHICLE_DATA vehicleData;
        vehicleData.index = positionHelper->getMemberPosition(pb->getVehicleId());
        vehicleData.acceleration = pb->getAcceleration();
        vehicleData.length = pb->getLength();
        vehicleData.positionX = pb->getPositionX();
        vehicleData.positionY = pb->getPositionY();
        vehicleData.speed = pb->getSpeed();
        vehicleData.time = pb->getTime();
        vehicleData.u = pb->getControllerAcceleration();
        vehicleData.speedX = pb->getSpeedX();
        vehicleData.speedY = pb->getSpeedY();
        vehicleData.angle = pb->getAngle();
        // send information to CACC controllers
        plexeTraciVehicle->setVehicleData(&vehicleData);
    }
    delete pb;
}

} // namespace plexe
