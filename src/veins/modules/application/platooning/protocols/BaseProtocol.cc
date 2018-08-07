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

#include "veins/modules/application/platooning/protocols/BaseProtocol.h"

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"

using namespace Veins;

Define_Module(BaseProtocol)

    // set signals for channel busy and collisions
    const simsignalwrap_t BaseProtocol::sigChannelBusy = simsignalwrap_t("sigChannelBusy");
const simsignalwrap_t BaseProtocol::sigCollision = simsignalwrap_t("sigCollision");

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
        packetSize = par("packetSize").longValue();
        // priority of platooning message
        priority = par("priority").longValue();
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
        findHost()->subscribe(sigChannelBusy, this);
        findHost()->subscribe(sigCollision, this);

        // init statistics collection. round to second
        SimTime rounded = SimTime(floor(simTime().dbl() + 1), SIMTIME_S);
        scheduleAt(rounded, recordData);
    }

    if (stage == 1) {
        // get traci interface
        mobility = Veins::TraCIMobilityAccess().get(getParentModule());
        ASSERT(mobility);
        traci = mobility->getCommandInterface();
        ASSERT(traci);
        traciVehicle = mobility->getVehicleCommandInterface();
        ASSERT(traciVehicle);
        positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());
        ASSERT(positionHelper);

        // this is the id of the vehicle. used also as network address
        myId = positionHelper->getId();
        length = traciVehicle->getLength();
        // tell the unicast protocol below which mac address to use via control message
        UnicastProtocolControlMessage* setMacAddress = new UnicastProtocolControlMessage("");
        setMacAddress->setControlCommand(SET_MAC_ADDRESS);
        setMacAddress->setCommandValue(myId);
        send(setMacAddress, lowerControlOut);
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

    // vehicle's data to be included in the message
    Plexe::VEHICLE_DATA data;
    // get information about the vehicle via traci
    traciVehicle->getVehicleData(&data);

    // create and send beacon
    UnicastMessage* unicast = new UnicastMessage("", BEACON_TYPE);
    unicast->setDestination(-1);
    unicast->setPriority(priority);
    unicast->setChannel(Channels::CCH);

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
    // i generated the message, i send it
    pkt->setRelayerId(myId);
    pkt->setKind(BEACON_TYPE);
    pkt->setByteLength(packetSize);
    pkt->setSequenceNumber(seq_n++);

    // put platooning beacon into the message for the UnicastProtocol
    unicast->encapsulate(pkt);
    sendDown(unicast);
}

void BaseProtocol::handleUnicastMsg(UnicastMessage* unicast)
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
        AppList::iterator i;
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
    if (signalID == sigChannelBusy) {
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
    if (signalID == sigCollision) {
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
    handleUnicastMsg(check_and_cast<UnicastMessage*>(msg));
}

void BaseProtocol::handleUpperMsg(cMessage* msg)
{
    sendDown(check_and_cast<UnicastMessage*>(msg));
}

void BaseProtocol::handleUpperControl(cMessage* msg)
{
    UnicastProtocolControlMessage* ctrl = dynamic_cast<UnicastProtocolControlMessage*>(msg);
    if (ctrl) {
        if (ctrl->getControlCommand() == SET_MAC_ADDRESS) {
            // set id to be the address we want to set to the NIC card
            myId = ctrl->getCommandValue();
        }
        sendControlDown(ctrl);
    }
}

void BaseProtocol::handleLowerControl(cMessage* msg)
{
    UnicastProtocolControlMessage* ctrl = dynamic_cast<UnicastProtocolControlMessage*>(msg);
    if (ctrl) {
        UnicastMessage* unicast = dynamic_cast<UnicastMessage*>(ctrl->getEncapsulatedPacket());
        if (unicast) {
            // find the application responsible for this beacon
            ApplicationMap::iterator app = apps.find(unicast->getKind());
            if (app != apps.end() && app->second.size() != 0) {
                AppList applications = app->second;
                AppList::iterator i;
                for (AppList::iterator i = applications.begin(); i != applications.end(); i++) {
                    // send the message to the applications responsible for it
                    send(ctrl->dup(), std::get<3>(*i));
                }
            }
        }
        delete ctrl;
    }
}

void BaseProtocol::messageReceived(PlatooningBeacon* pkt, UnicastMessage* unicast)
{
    ASSERT2(false, "BaseProtocol::messageReceived() not overridden by subclass");
}

void BaseProtocol::registerApplication(int applicationId, InputGate* appInputGate, OutputGate* appOutputGate, ControlInputGate* appControlInputGate, ControlOutputGate* appControlOutputGate)
{
    if (usedGates == MAX_GATES_COUNT) throw cRuntimeError("BaseProtocol: application with id=%d tried to register, but no space left", applicationId);
    // connect gates, if not already connected. a gate might be already
    // connected if an application is registering for multiple packet types
    cGate *upperIn, *upperOut, *upperCntIn, *upperCntOut;
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
