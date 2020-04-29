//
// Copyright (C) 2012-2020 Michele Segata <segata@ccs-labs.org>
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

#include "VlcRepropagationProtocol.h"
#include "plexe/driver/PlexeRadioDriverInterface.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"
#include "plexe_vlc/VeinsVLCRadioDriver.h"
#include "veins/base/utils/FindModule.h"

namespace plexe {

Define_Module(VlcRepropagationProtocol)

void VlcRepropagationProtocol::initialize(int stage)
{
    BaseProtocol::initialize(stage);

    if (stage == 0) {
        // random start time
        SimTime beginTime = SimTime(uniform(0.001, beaconingInterval));
        if (beaconingInterval > 0) scheduleAt(simTime() + beaconingInterval + beginTime, sendBeacon);
        return;
    }

    if (stage == 1) {
        if (VeinsVLCRadioDriver* driver = veins::FindModule<VeinsVLCRadioDriver*>::findSubModule(getParentModule())) {
            driver->registerNode(myId);
        }
    }
}

void VlcRepropagationProtocol::handleSelfMsg(cMessage* msg)
{

    BaseProtocol::handleSelfMsg(msg);

    if (msg == sendBeacon) {
        sendPlatooningMessage(-1);
        scheduleAt(simTime() + beaconingInterval, sendBeacon);
    }
    else if (BaseFrame1609_4* frame = dynamic_cast<BaseFrame1609_4*>(msg)) {
        PlexeInterfaceControlInfo* itf = dynamic_cast<PlexeInterfaceControlInfo*>(frame->getControlInfo());
        sendTo(frame, (enum PlexeRadioInterfaces)itf->getInterfaces());
    }
}

bool VlcRepropagationProtocol::updateAndCheckRepropagation(PlatooningBeacon* pkt)
{
    int senderId = pkt->getVehicleId();
    auto seqNumber = seqNumbers.find(senderId);
    if (seqNumber == seqNumbers.end()) {
        seqNumbers[senderId] = pkt->getSequenceNumber();
        return true;
    }
    else {
        if (pkt->getSequenceNumber() > seqNumber->second) {
            seqNumbers[senderId] = pkt->getSequenceNumber();
            return true;
        }
        else {
            return false;
        }
    }
}

void VlcRepropagationProtocol::messageReceived(PlatooningBeacon* pkt, veins::BaseFrame1609_4* frame)
{

    int senderId = pkt->getVehicleId();

    // if the sender is myself, this is an already re-propagated message
    if (senderId == myId) return;

    PlexeInterfaceControlInfo* itf = dynamic_cast<PlexeInterfaceControlInfo*>(frame->getControlInfo());
    int interfaces = VEINS_VLC_FRONT | VEINS_VLC_BACK;
    if (itf) {
        if (itf->getInterfaces() == (VEINS_VLC_FRONT | VEINS_VLC_BACK)) throw cRuntimeError("The VLC driver indicates a frame being received from both head and tail lights");
        // if it is coming from the front, send it to the back
        if (itf->getInterfaces() & VEINS_VLC_FRONT) {
            interfaces = VEINS_VLC_BACK;
        }
        // if it is coming from the back, send it to the front
        if (itf->getInterfaces() & VEINS_VLC_BACK) {
            interfaces = VEINS_VLC_FRONT;
        }
    }

    // the leader and the last car don't need to re-propagate
    if (!positionHelper->isLeader() && !positionHelper->isLast()) {
        // if it is the first time we see this packet
        if (updateAndCheckRepropagation(pkt)) {
            // make a copy of the frame and send it
            BaseFrame1609_4* repropagatedFrame = frame->dup();
            PlexeInterfaceControlInfo* ctrl = new PlexeInterfaceControlInfo();
            ctrl->setInterfaces(interfaces);
            repropagatedFrame->setControlInfo(ctrl);
            // send the frame with a random delay to avoid collisions, as there is no real MAC protocol in VLC
            scheduleAt(simTime() + SimTime(uniform(0, 0.02)), repropagatedFrame);
        }
    }

}

VlcRepropagationProtocol::VlcRepropagationProtocol()
{
}

VlcRepropagationProtocol::~VlcRepropagationProtocol()
{
}

} // namespace plexe
