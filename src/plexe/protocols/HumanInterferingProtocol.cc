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

#include "plexe/protocols/HumanInterferingProtocol.h"

#include "plexe/messages/InterferingBeacon_m.h"
#include "veins/modules/messages/PhyControlMessage_m.h"
#include "plexe/driver/Veins11pRadioDriver.h"

using namespace veins;

namespace plexe {

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
        mobility = veins::TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        // get pointer to mac
        mac = FindModule<Mac1609_4*>::findSubModule(getParentModule());

        if (Veins11pRadioDriver* driver = FindModule<Veins11pRadioDriver*>::findSubModule(getParentModule())) {
            driver->registerNode(getParentModule()->getIndex() + 1e6);
        }

        // beaconing interval in seconds
        beaconingInterval = SimTime(par("beaconingInterval").doubleValue());
        // platooning message packet size
        packetSize = par("packetSize");
        // priority of platooning message
        priority = par("priority");
        ASSERT2(priority >= 0 && priority <= 7, "priority value must be between 0 and 7");
        // tx power
        txPower = par("txPower").doubleValue();
        // bit rate
        bitrate = par("bitrate");

        // init messages for scheduleAt
        sendBeacon = new cMessage("sendBeacon");
        scheduleAt(simTime() + uniform(0, beaconingInterval), sendBeacon);
    }

    if (stage == 1) {
        // METHOD 1: setting tx power and bitrate for all frames sent
        // call this method at stage 1 otherwise the MAC might overwrite the
        // values with the ones loaded from omnetpp.ini
        mac->setTxPower(txPower);
        mac->setMCS(getMCS(bitrate, Bandwidth::ofdm_10_mhz));
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
    BaseFrame1609_4* unicast = new BaseFrame1609_4("", INTERFERENCE_TYPE);
    unicast->setRecipientAddress(LAddress::L2BROADCAST());
    unicast->setUserPriority(priority);
    unicast->setChannelNumber(static_cast<int>(Channel::cch));

    // create platooning beacon with data about the car
    InterferingBeacon* pkt = new InterferingBeacon();
    pkt->setKind(INTERFERENCE_TYPE);
    pkt->setByteLength(packetSize);

    // METHOD 2: setting tx power and bitrate on a per frame basis
    PhyControlMessage* ctrl = new PhyControlMessage();
    ctrl->setTxPower_mW(txPower);
    ctrl->setMcs(static_cast<int>(getMCS(bitrate, Bandwidth::ofdm_10_mhz)));
    pkt->setControlInfo(ctrl);

    // put platooning beacon into the message for the UnicastProtocol
    unicast->encapsulate(pkt);
    sendDown(unicast);
}

void HumanInterferingProtocol::handleLowerMsg(cMessage* msg)
{
    delete msg;
}

} // namespace plexe
