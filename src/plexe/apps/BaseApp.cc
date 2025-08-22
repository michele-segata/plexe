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

#include "plexe/messages/PlexeInterfaceControlInfo_m.h"

using namespace veins;

namespace plexe {

Define_Module(BaseApp);

void BaseApp::initialize(int stage)
{

    BaseApplLayer::initialize(stage);

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
    }
}

void BaseApp::finish()
{
    recordScalar("crashed", crashed);
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
    delete msg;
}

void BaseApp::logVehicleData(bool crashed)
{
    // get distance and relative speed w.r.t. front vehicle
    double distance, relSpeed;
    VEHICLE_DATA data;
    plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
    plexeTraciVehicle->getVehicleData(&data);
    if (crashed) {
        this->crashed = true;
        distance = 0;
        this->crashed = true;
        stopSimulation = new cMessage("stopSimulation");
        scheduleAt(simTime() + SimTime(1, SIMTIME_MS), stopSimulation);
    }
    // write data to output files
    distanceOut.record(distance);
    relSpeedOut.record(relSpeed);
    nodeIdOut.record(myId);
    accelerationOut.record(data.acceleration);
    controllerAccelerationOut.record(data.u);
    speedOut.record(data.speed);
    posxOut.record(data.positionX);
    posyOut.record(data.positionY);
}

void BaseApp::handleLowerControl(cMessage* msg)
{
    delete msg;
}

void BaseApp::sendFrame(cPacket* msg, int destination, short type, enum PlexeRadioInterfaces interfaces)
{
    // add interfaces to be used to send message as control information
    PlexeInterfaceControlInfo* ctlInfo = new PlexeInterfaceControlInfo();
    ctlInfo->setInterfaces((int)interfaces);

    BaseFrame1609_4* frame = new BaseFrame1609_4();
    frame->setRecipientAddress(destination);
    // set kind to both the encapsulated packet and the outer frame
    frame->setKind(type);
    msg->setKind(type);
    frame->encapsulate(msg);
    frame->setChannelNumber(static_cast<int>(Channel::cch));
    frame->setControlInfo(ctlInfo);
    sendDown(frame);
}

void BaseApp::handleSelfMsg(cMessage* msg)
{
    if (msg == recordData) {
        // log mobility data
        logVehicleData(plexeTraciVehicle->isCrashed());
        // re-schedule next event
        scheduleAt(simTime() + SimTime(100, SIMTIME_MS), recordData);
    }
    if (msg == stopSimulation) {
        endSimulation();
    }
}

void BaseApp::enableLogging()
{
    // set names for output vectors
    // distance from front vehicle
    distanceOut.setName("distance");
    // relative speed w.r.t. front vehicle
    relSpeedOut.setName("relativeSpeed");
    // vehicle id
    nodeIdOut.setName("nodeId");
    // current speed
    speedOut.setName("speed");
    // vehicle position
    posxOut.setName("posx");
    posyOut.setName("posy");
    // vehicle acceleration
    accelerationOut.setName("acceleration");
    controllerAccelerationOut.setName("controllerAcceleration");

    recordData = new cMessage("recordData");
    // init statistics collection. round to 0.1 seconds
    SimTime rounded = SimTime(floor(simTime().dbl() * 1000 + 100), SIMTIME_MS);
    scheduleAt(rounded, recordData);
}

} // namespace plexe
