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

#include "SlottedBeaconing.h"

namespace plexe {

Define_Module(SlottedBeaconing)

void SlottedBeaconing::initialize(int stage)
{
    BaseProtocol::initialize(stage);

    if (stage == 1) {

        int positionInPlatoon = positionHelper->getPosition();
        // one beacon interval is divided into 'platoonSize' slots
        slotNumber = positionInPlatoon;
        slotTime = SimTime(slotNumber * beaconingInterval / positionHelper->getPlatoonSize());

        // only the leader starts to communicate. the followers use
        // the slotted approach, i.e., they compute their sending time
        // based on their position
        if (positionHelper->isLeader()) {
            // random start time
            SimTime beginTime = SimTime(uniform(0.001, 1.0));
            scheduleAt(simTime() + beaconingInterval + beginTime, sendBeacon);
        }
    }
}

void SlottedBeaconing::handleSelfMsg(cMessage* msg)
{

    BaseProtocol::handleSelfMsg(msg);

    if (msg == sendBeacon) {
        sendPlatooningMessage(0);
        // we might not be able to receive a message from the leader, so we schedule the next beacon
        // after one beacon interval. if we will receive the message from the leader, we will
        // delete this event and synchronize on the beacon of the leader
        scheduleAt(simTime() + beaconingInterval, sendBeacon);
    }
}

void SlottedBeaconing::messageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* unicast)
{

    int senderId = pkt->getVehicleId();

    if (positionHelper->getLeaderId() == senderId && !positionHelper->isLast()) {
        // we received a message from the leader. use that for synchronization

        if (sendBeacon->isScheduled()) {
            // we were scheduling a beacon, but we received a message from the leader
            // we can use that for synchronization
            cancelEvent(sendBeacon);
        }

        scheduleAt(simTime() + slotTime, sendBeacon);
    }
}

SlottedBeaconing::SlottedBeaconing()
{
}

SlottedBeaconing::~SlottedBeaconing()
{
}

} // namespace plexe
