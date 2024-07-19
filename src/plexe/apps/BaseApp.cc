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

#include "plexe/apps/BaseApp.h"

#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/base/messages/MacPkt_m.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "veins/base/utils/FindModule.h"

#include "plexe/protocols/BaseProtocol.h"
#include "plexe/PlexeManager.h"

using namespace veins;

namespace plexe {

Define_Module(BaseApp);

void BaseApp::initialize(int stage)
{

    BaseApplLayer::initialize(stage);

    if (stage == 0) {

        // registering signals emitted to record nodeId, distance, speed...
        // id of the vehicle
        nodeIdSignal = registerSignal("nodeId");
        // distance and relative speed
        distanceSignal = registerSignal("distance");
        relativeSpeedSignal = registerSignal("relativeSpeed");
        // speed and position
        speedSignal = registerSignal("speed");
        posxSignal = registerSignal("posx");
        posySignal = registerSignal("posy");
        // real acceleration and controller acceleration
        accelerationSignal = registerSignal("acceleration");
        controllerAccelerationSignal = registerSignal("controllerAcceleration");

        enableLogging = par("enableLogging").boolValue();
        loggingInterval = par("loggingInterval").doubleValue();
    }

    if (stage == 1) {
        mobility = veins::TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        auto plexe = FindModule<PlexeManager*>::findGlobalModule();
        ASSERT(plexe);
        plexeTraci = plexe->getCommandInterface();
        plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, mobility->getExternalId()));
        positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());
        protocol = FindModule<BaseProtocol*>::findSubModule(getParentModule());
        myId = positionHelper->getId();

        // connect application to protocol
        protocol->registerApplication(BaseProtocol::BEACON_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));

        recordData = new cMessage("recordData");
        // init statistics collection. round to 0.1 seconds
        SimTime rounded = SimTime(floor(simTime().dbl() * 1000 + 100), SIMTIME_MS);
        scheduleAt(rounded, recordData);
    }
}

BaseApp::~BaseApp()
{
    cancelAndDelete(recordData);
    recordData = nullptr;
    cancelAndDelete(stopSimulation);
    stopSimulation = nullptr;
}

void BaseApp::handleLowerMsg(cMessage* msg)
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

void BaseApp::logVehicleData(bool crashed)
{
    // get distance and relative speed w.r.t. front vehicle
    double distance, relSpeed;
    VEHICLE_DATA data;
    plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
    plexeTraciVehicle->getVehicleData(&data);
    if (crashed) {
        distance = 0;
        stopSimulation = new cMessage("stopSimulation");
        scheduleAt(simTime() + SimTime(1, SIMTIME_MS), stopSimulation);
    }

    // emit signals!
    emit(nodeIdSignal, myId);
    emit(distanceSignal, distance);
    emit(relativeSpeedSignal, relSpeed);
    emit(speedSignal, data.speed);
    emit(posxSignal, data.positionX);
    emit(posySignal, data.positionY);
    emit(accelerationSignal, data.acceleration);
    emit(controllerAccelerationSignal, data.u);
}

void BaseApp::handleLowerControl(cMessage* msg)
{
    delete msg;
}

void BaseApp::sendFrame(cPacket* msg, int destination)
{
    BaseFrame1609_4* frame = new BaseFrame1609_4();
    frame->setRecipientAddress(destination);
    frame->encapsulate(msg);
    sendDown(frame);
}

void BaseApp::handleSelfMsg(cMessage* msg)
{
    if (msg == recordData) {
        // log mobility data
        if (enableLogging) {
            logVehicleData(plexeTraciVehicle->isCrashed());
        }
        else {
            if (plexeTraciVehicle->isCrashed()) {
                stopSimulation = new cMessage("stopSimulation");
                scheduleAt(simTime() + SimTime(1, SIMTIME_MS), stopSimulation);
            }
        }
        // re-schedule next event
        scheduleAt(simTime() + loggingInterval, recordData);
    }
    if (msg == stopSimulation) {
        throw cRuntimeError("Collision detected!");
        //endSimulation();
    }
}

void BaseApp::onPlatoonBeacon(const PlatooningBeacon* pb)
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
        // send data about every vehicle to the CACC. this is needed by the consensus controller
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
        // send information to CACC
        plexeTraciVehicle->setVehicleData(&vehicleData);
    }
    delete pb;
}

} // namespace plexe
