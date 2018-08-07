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

#include "veins/modules/application/platooning/protocols/HumanInterferingProtocol.h"

#include "veins/modules/application/platooning/messages/InterferingBeacon_m.h"
#include "veins/modules/messages/PhyControlMessage_m.h"

using namespace Veins;

Define_Module(HumanInterferingProtocol)

    void HumanInterferingProtocol::initialize(int stage)
{

    BaseApplLayer::initialize(stage);

    if (stage == 0) {

        // init class variables
        sendBeacon = 0;

        // get gates
        lowerLayerIn = findGate("lowerLayerIn");
        lowerLayerOut = findGate("lowerLayerOut");

        // get traci interface
        mobility = Veins::TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        // get pointer to mac
        mac = FindModule<Mac1609_4*>::findSubModule(getParentModule());

        // tell the unicast protocol below which mac address to use via control message
        UnicastProtocolControlMessage* setMacAddress = new UnicastProtocolControlMessage("");
        setMacAddress->setControlCommand(SET_MAC_ADDRESS);
        // set a mac address not interfering with platooning vehicles
        setMacAddress->setCommandValue(getParentModule()->getIndex() + 1e6);
        send(setMacAddress, lowerControlOut);

        // beaconing interval in seconds
        beaconingInterval = SimTime(par("beaconingInterval").doubleValue());
        // platooning message packet size
        packetSize = par("packetSize").longValue();
        // priority of platooning message
        priority = par("priority").longValue();
        ASSERT2(priority >= 0 && priority <= 7, "priority value must be between 0 and 7");
        // tx power
        txPower = par("txPower").doubleValue();
        // bit rate
        bitrate = par("bitrate").doubleValue();

        // init messages for scheduleAt
        sendBeacon = new cMessage("sendBeacon");
        scheduleAt(simTime() + uniform(0, beaconingInterval), sendBeacon);
    }

    if (stage == 1) {
        // METHOD 1: setting tx power and bitrate for all frames sent
        // call this method at stage 1 otherwise the MAC might overwrite the
        // values with the ones loaded from omnetpp.ini
        mac->setTxPower(txPower);
        mac->setMCS(getMCS(bitrate, BW_OFDM_10_MHZ));
    }
}

HumanInterferingProtocol::~HumanInterferingProtocol()
{
    cancelAndDelete(sendBeacon);
    sendBeacon = nullptr;
}

void HumanInterferingProtocol::handleSelfMsg(cMessage* msg)
{
    if (msg == sendBeacon) {
        sendInterferingMessage();
        scheduleAt(simTime() + beaconingInterval, sendBeacon);
    }
}

void HumanInterferingProtocol::sendInterferingMessage()
{

    // create and send beacon
    UnicastMessage* unicast = new UnicastMessage("", INTERFERENCE_TYPE);
    unicast->setDestination(-1);
    unicast->setPriority(priority);
    unicast->setChannel(Channels::CCH);

    // create platooning beacon with data about the car
    InterferingBeacon* pkt = new InterferingBeacon();
    pkt->setKind(INTERFERENCE_TYPE);
    pkt->setByteLength(packetSize);

    // METHOD 2: setting tx power and bitrate on a per frame basis
    PhyControlMessage* ctrl = new PhyControlMessage();
    ctrl->setTxPower_mW(txPower);
    ctrl->setMcs(getMCS(bitrate, BW_OFDM_10_MHZ));
    pkt->setControlInfo(ctrl);

    // put platooning beacon into the message for the UnicastProtocol
    unicast->encapsulate(pkt);
    sendDown(unicast);
}

void HumanInterferingProtocol::handleLowerMsg(cMessage* msg)
{
    delete msg;
}
