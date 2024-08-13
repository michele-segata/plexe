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
// Adapted from AlertSender from SimuLTE
//

#include "LTECV2XMode3RadioDriver.h"
#include "inet/common/ModuleAccess.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "plexe_5g/PlexeInetUtils.h"

using namespace veins;

namespace plexe {

Define_Module(LTECV2XMode3RadioDriver);

LTECV2XMode3RadioDriver::LTECV2XMode3RadioDriver()
    : destinationPort(3000)
    , socketInGate(-1)
    , socketOutGate(-1)
    , upperLayerIn(-1)
    , upperLayerOut(-1)
{}

LTECV2XMode3RadioDriver::~LTECV2XMode3RadioDriver()
{}

void LTECV2XMode3RadioDriver::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    socketInGate = findGate("socketIn");
    socketOutGate = findGate("socketOut");
    upperLayerIn = findGate("upperLayerIn");
    upperLayerOut = findGate("upperLayerOut");

    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    destinationPort = par("destinationPort");
    socket.setOutputGate(gate("socketOut"));
    socket.bind(destinationPort);
    setMulticastAddress(par("multicastAddress").stdstringValue());
}

void LTECV2XMode3RadioDriver::setMulticastAddress(std::string address)
{
    if (!multicastAddress.isUnspecified()) {
        // we are already bound to a multicast address. leave this group first
        socket.leaveMulticastGroup(multicastAddress);
    }
    multicastAddress = inet::L3AddressResolver().resolve(address.c_str());
    inet::IInterfaceTable* ift = inet::getModuleFromPar<inet::IInterfaceTable>(par("interfaceTableModule"), this);
    inet::NetworkInterface* ie = ift->findInterfaceByName("cellular");
    if (!ie)
        throw cRuntimeError("Wrong multicastInterface setting: no interface named wlan");
    socket.setMulticastOutputInterface(ie->getInterfaceId());
    socket.joinMulticastGroup(multicastAddress, ie->getInterfaceId());
}

void LTECV2XMode3RadioDriver::handleMessage(cMessage* msg)
{
    if (msg->getArrivalGateId() == upperLayerIn) {
        BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);
        inet::Packet* container = PlexeInetUtils::encapsulate(frame, "BaseFrame1609_4_Container");
        socket.sendTo(container, multicastAddress, destinationPort);
    }
    else if (msg->getArrivalGateId() == socketInGate) {
        inet::Packet* container = check_and_cast<inet::Packet*>(msg);
        BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(PlexeInetUtils::decapsulate(container));
        send(frame, upperLayerOut);
        delete container;
    }
}

} // namespace plexe
