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

#include "plexe/apps/HelloPlexeApp.h"

#include "plexe/protocols/BaseProtocol.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "plexe/messages/HelloPlexeMsg_m.h"
#include "veins/modules/utility/Consts80211p.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"

namespace plexe {

Define_Module(HelloPlexeApp);

#define HELLO_PLEXE_TYPE 9876

void HelloPlexeApp::initialize(int stage)
{
    BaseApp::initialize(stage);

    if (stage == 1) {
        // connect this app application to protocol, and specify we're interested in receiving HELLO_PLEXE_TYPE messages
        protocol->registerApplication(HELLO_PLEXE_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));

        senderVehicleId = par("senderVehicleId");
        // let's assume vehicle with id 0 is the only one sending an hello
        if (positionHelper->getId() == senderVehicleId) {
            sendMessageAfter = par("sendMessageAfter");
            sendHello = new cMessage("sendHello");
            // schedule sending after 3 seconds
            scheduleAfter(SimTime(sendMessageAfter), sendHello);
        }

        interfaceToUse = PlexeRadioDriverInterface::parseRadioInterface(par("interfaceToUse"));
    }
}

void HelloPlexeApp::finish()
{
    cancelAndDelete(sendHello);
}

void HelloPlexeApp::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);

    if (frame->getKind() == HELLO_PLEXE_TYPE) {
        HelloPlexeMsg* hello = check_and_cast<HelloPlexeMsg*>(frame->decapsulate());
        EV << "HelloPlexeApp received " << hello->getText() << " from " << hello->getSenderVehicle() << "\n";
        auto incomingInterface = dynamic_cast<PlexeInterfaceControlInfo*>(frame->getControlInfo());
        if (incomingInterface)
            EV << "HelloMessage received from interface " << PlexeRadioDriverInterface::radioInterfacesToString((enum PlexeRadioInterfaces) incomingInterface->getInterfaces()) << "\n";
        delete hello;
        delete frame;
    }
    else {
        BaseApp::handleLowerMsg(msg);
    }
}
void HelloPlexeApp::handleSelfMsg(cMessage* msg)
{
    if (msg == sendHello) {
        // create and send hello message
        HelloPlexeMsg* hello = new HelloPlexeMsg("HelloPlexeMsg");
        hello->setSenderVehicle(positionHelper->getId());
        hello->setText("Hello World of Plexe");
        sendFrame(hello, -1, HELLO_PLEXE_TYPE, interfaceToUse);
        EV << "Vehicle " << positionHelper->getId() << " sending hello message\n";
    }
    else {
        BaseApp::handleSelfMsg(msg);
    }
}

} // namespace plexe
