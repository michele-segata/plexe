//
// Copyright (C) 2012-2019 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/protocols/BaseProtocol.h"

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "veins/base/utils/FindModule.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"

#include "plexe/PlexeManager.h"
#include "plexe/driver/Veins11pRadioDriver.h"

using namespace veins;

namespace plexe {

Define_Module(BaseProtocol);


const int BaseProtocol::BEACON_TYPE = 12345;

void BaseProtocol::initialize(int stage)
{

    BaseApplLayer::initialize(stage);

    if (stage == 0) {

        // init class variables
        sendBeacon = 0;
        channelBusy = false;
        nCollisions = 0;
        busyTime = SimTime(0);
        seq_n = 0;
        recordData = 0;

        // get gates
        lowerControlIn = findGate("lowerControlIn");
        lowerControlOut = findGate("lowerControlOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerLayerOut = findGate("lowerLayerOut");
        minUpperId = gate("upperLayerIn", 0)->getId();
        maxUpperId = gate("upperLayerIn", MAX_GATES_COUNT - 1)->getId();
        minUpperControlId = gate("upperControlIn", 0)->getId();
        maxUpperControlId = gate("upperControlIn", MAX_GATES_COUNT - 1)->getId();

        // beaconing interval in seconds
        beaconingInterval = SimTime(par("beaconingInterval").doubleValue());
        // platooning message packet size
        packetSize = par("packetSize");
        // priority of platooning message
        priority = par("priority");
        ASSERT2(priority >= 0 && priority <= 7, "priority value must be between 0 and 7");

        // init messages for scheduleAt
        sendBeacon = new cMessage("sendBeacon");
        recordData = new cMessage("recordData");

        // set names for output vectors
        // own id
        nodeIdOut.setName("nodeId");
        // channel busy time
        busyTimeOut.setName("busyTime");
        // mac layer collisions
        collisionsOut.setName("collisions");
        // delay metrics
        lastLeaderMsgTime = SimTime(-1);
        lastFrontMsgTime = SimTime(-1);
        leaderDelayIdOut.setName("leaderDelayId");
        frontDelayIdOut.setName("frontDelayId");
        leaderDelayOut.setName("leaderDelay");
        frontDelayOut.setName("frontDelay");

        // subscribe to signals for channel busy state and collisions
        findHost()->subscribe(veins::Mac1609_4::sigChannelBusy, this);
        findHost()->subscribe(veins::Mac1609_4::sigCollision, this);

        // init statistics collection. round to second
        SimTime rounded = SimTime(floor(simTime().dbl() + 1), SIMTIME_S);
        scheduleAt(rounded, recordData);
    }

    if (stage == 1) {
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

        // this is the id of the vehicle. used also as network address
        myId = positionHelper->getId();
        length = traciVehicle->getLength();
        if (Veins11pRadioDriver* driver = FindModule<Veins11pRadioDriver*>::findSubModule(getParentModule())) {
            driver->registerNode(myId);
        }
    }
}

BaseProtocol::~BaseProtocol()
{
    cancelAndDelete(sendBeacon);
    sendBeacon = nullptr;
    cancelAndDelete(recordData);
    recordData = nullptr;
}

void BaseProtocol::handleSelfMsg(cMessage* msg)
{

    if (msg == recordData) {

        // if channel is currently busy, we have to split the amount of time between
        // this period and the successive. so we just compute the channel busy time
        // up to now, and then reset the "startBusy" timer to now
        if (channelBusy) {
            busyTime += simTime() - startBusy;
            startBusy = simTime();
        }

        // time for writing statistics
        // node id
        nodeIdOut.record(myId);
        // record busy time for this period
        busyTimeOut.record(busyTime);
        // record collisions for this period
        collisionsOut.record(nCollisions);

        // and reset counter
        busyTime = SimTime(0);
        nCollisions = 0;

        scheduleAt(simTime() + SimTime(1, SIMTIME_S), recordData);
    }
}

void BaseProtocol::sendPlatooningMessage(int destinationAddress)
{
    sendDown(createBeacon(destinationAddress).release());
}

std::unique_ptr<BaseFrame1609_4> BaseProtocol::createBeacon(int destinationAddress)
{
    // vehicle's data to be included in the message
    VEHICLE_DATA data;
    // get information about the vehicle via traci
    plexeTraciVehicle->getVehicleData(&data);

    // create and send beacon
    auto wsm = veins::make_unique<BaseFrame1609_4>("", BEACON_TYPE);
    wsm->setRecipientAddress(LAddress::L2BROADCAST());
    wsm->setChannelNumber(static_cast<int>(Channel::cch));
    wsm->setUserPriority(priority);

    // create platooning beacon with data about the car
    PlatooningBeacon* pkt = new PlatooningBeacon();
    pkt->setControllerAcceleration(data.u);
    pkt->setAcceleration(data.acceleration);
    pkt->setSpeed(data.speed);
    pkt->setVehicleId(myId);
    pkt->setPositionX(data.positionX);
    pkt->setPositionY(data.positionY);
    // set the time to now
    pkt->setTime(data.time);
    pkt->setLength(length);
    pkt->setSpeedX(data.speedX);
    pkt->setSpeedY(data.speedY);
    pkt->setAngle(data.angle);
    pkt->setKind(BEACON_TYPE);
    pkt->setByteLength(packetSize);
    pkt->setSequenceNumber(seq_n++);

    wsm->encapsulate(pkt);

    return wsm;
}

void BaseProtocol::handleUnicastMsg(BaseFrame1609_4* unicast)
{

    ASSERT2(unicast, "received a frame not of type UnicastMessage");

    cPacket* enc = unicast->getEncapsulatedPacket();
    ASSERT2(enc, "received a UnicastMessage with nothing inside");

    if (PlatooningBeacon* epkt = dynamic_cast<PlatooningBeacon*>(enc)) {

        // invoke messageReceived() method of subclass
        messageReceived(epkt, unicast);

        if (positionHelper->getLeaderId() == epkt->getVehicleId()) {
            // check if this is at least the second message we have received
            if (lastLeaderMsgTime.dbl() > 0) {
                leaderDelayOut.record(simTime() - lastLeaderMsgTime);
                leaderDelayIdOut.record(myId);
            }
            lastLeaderMsgTime = simTime();
        }
        if (positionHelper->getFrontId() == epkt->getVehicleId()) {
            // check if this is at least the second message we have received
            if (lastFrontMsgTime.dbl() > 0) {
                frontDelayOut.record(simTime() - lastFrontMsgTime);
                frontDelayIdOut.record(myId);
            }
            lastFrontMsgTime = simTime();
        }
    }

    // find the application responsible for this beacon
    ApplicationMap::iterator app = apps.find(unicast->getKind());
    if (app != apps.end() && app->second.size() != 0) {
        AppList applications = app->second;
        for (AppList::iterator i = applications.begin(); i != applications.end(); i++) {
            // send the message to the applications responsible for it
            send(unicast->dup(), std::get<1>(*i));
        }
    }
    delete unicast;
}

void BaseProtocol::receiveSignal(cComponent* source, simsignal_t signalID, bool v, cObject* details)
{

    Enter_Method_Silent();
    if (signalID == veins::Mac1609_4::sigChannelBusy) {
        if (v && !channelBusy) {
            // channel turned busy, was idle before
            startBusy = simTime();
            channelBusy = true;
            channelBusyStart();
            return;
        }
        if (!v && channelBusy) {
            // channel turned idle, was busy before
            busyTime += simTime() - startBusy;
            channelBusy = false;
            channelIdleStart();
            return;
        }
    }
    if (signalID == veins::Mac1609_4::sigCollision) {
        collision();
        nCollisions++;
    }
}

void BaseProtocol::handleMessage(cMessage* msg)
{
    if (msg->getArrivalGateId() >= minUpperId && msg->getArrivalGateId() <= maxUpperId)
        handleUpperMsg(msg);
    else if (msg->getArrivalGateId() >= minUpperControlId && msg->getArrivalGateId() <= maxUpperControlId)
        handleUpperControl(msg);
    else
        BaseApplLayer::handleMessage(msg);
}

void BaseProtocol::handleLowerMsg(cMessage* msg)
{
    handleUnicastMsg(check_and_cast<BaseFrame1609_4*>(msg));
}

void BaseProtocol::handleUpperMsg(cMessage* msg)
{
    sendDown(check_and_cast<BaseFrame1609_4*>(msg));
}

void BaseProtocol::messageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* unicast)
{
}

void BaseProtocol::registerApplication(int applicationId, InputGate* appInputGate, OutputGate* appOutputGate, ControlInputGate* appControlInputGate, ControlOutputGate* appControlOutputGate)
{
    if (usedGates == MAX_GATES_COUNT) throw cRuntimeError("BaseProtocol: application with id=%d tried to register, but no space left", applicationId);
    // connect gates, if not already connected. a gate might be already
    // connected if an application is registering for multiple packet types
    cGate* upperIn;
    cGate* upperOut;
    cGate* upperCntIn;
    cGate* upperCntOut;
    if (!appInputGate->isConnected() || !appOutputGate->isConnected() || !appControlInputGate->isConnected() || !appControlOutputGate->isConnected()) {
        if (appInputGate->isConnected() || appOutputGate->isConnected() || appControlInputGate->isConnected() || appControlOutputGate->isConnected()) throw cRuntimeError("BaseProtocol: the application should not be connected but one of its gates is connected");
        upperOut = gate("upperLayerOut", usedGates);
        upperOut->connectTo(appInputGate);
        upperIn = gate("upperLayerIn", usedGates);
        appOutputGate->connectTo(upperIn);
        connections[appInputGate] = upperOut;
        connections[appOutputGate] = upperIn;
        upperCntOut = gate("upperControlOut", usedGates);
        upperCntOut->connectTo(appControlInputGate);
        upperCntIn = gate("upperControlIn", usedGates);
        appControlOutputGate->connectTo(upperCntIn);
        connections[appControlInputGate] = upperCntOut;
        connections[appControlOutputGate] = upperCntIn;
        usedGates++;
    }
    else {
        // find BaseProtocol gates already connected to the application
        GateConnections::iterator gate;
        gate = connections.find(appOutputGate);
        if (gate == connections.end()) throw cRuntimeError("BaseProtocol: gate should already be connected by not found in the connection list");
        upperIn = gate->second;
        gate = connections.find(appInputGate);
        if (gate == connections.end()) throw cRuntimeError("BaseProtocol: gate should already be connected by not found in the connection list");
        upperOut = gate->second;
        gate = connections.find(appControlOutputGate);
        if (gate == connections.end()) throw cRuntimeError("BaseProtocol: gate should already be connected by not found in the connection list");
        upperCntIn = gate->second;
        gate = connections.find(appControlInputGate);
        if (gate == connections.end()) throw cRuntimeError("BaseProtocol: gate should already be connected by not found in the connection list");
        upperCntOut = gate->second;
    }
    // save the mapping in the connection
    apps[applicationId].push_back(AppInOut(upperIn, upperOut, upperCntIn, upperCntOut));
}

} // namespace plexe
