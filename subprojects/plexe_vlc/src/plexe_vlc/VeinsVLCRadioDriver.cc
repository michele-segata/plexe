//
// Copyright (C) 2020 Michele Segata <segata@ccs-labs.org>
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
//

#include "VeinsVLCRadioDriver.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "veins-vlc/mac/MacLayerVlc.h"
#include "veins-vlc/messages/VlcMessage_m.h"
#include "veins/base/utils/FindModule.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"
#include "plexe/CC_Const.h"


#define VEH_ID_TO_MAC(x) (x+1)
#define MAC_TO_VEH_ID(x) (x-1)

using namespace veins;

namespace plexe {

Define_Module(plexe::VeinsVLCRadioDriver);

void VeinsVLCRadioDriver::handleLowerMsg(cMessage* msg)
{
    VlcMessage* vlcMsg = check_and_cast<VlcMessage*>(msg);
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(vlcMsg->decapsulate());
    if (frame->getRecipientAddress() != veins::LAddress::L2BROADCAST()) frame->setRecipientAddress(MAC_TO_VEH_ID(frame->getRecipientAddress()));
    PlexeInterfaceControlInfo* itf = new PlexeInterfaceControlInfo();
    if (vlcMsg->getTransmissionModule() == HEADLIGHT) {
        itf->setInterfaces(VEINS_VLC_FRONT);
        LOG << nodeId << " Received frame from the head light\n";
    }
    else {
        itf->setInterfaces(VEINS_VLC_BACK);
        LOG << nodeId << " Received frame from the tail light\n";
    }
    frame->setControlInfo(itf);
    sendUp(frame);
    delete vlcMsg;
}

void VeinsVLCRadioDriver::handleUpperMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);
    PlexeInterfaceControlInfo* itf = dynamic_cast<PlexeInterfaceControlInfo*>(frame->getControlInfo());
    if (frame->getRecipientAddress() != veins::LAddress::L2BROADCAST()) frame->setRecipientAddress(VEH_ID_TO_MAC(frame->getRecipientAddress()));
    std::unique_ptr<VlcMessage> vlcMsg = veins::make_unique<VlcMessage>("", frame->getKind());
    vlcMsg->setAccessTechnology(VLC);

    int interfaces = VEINS_VLC_FRONT | VEINS_VLC_BACK;
    if (itf) interfaces = itf->getInterfaces();
    switch (interfaces) {
    case VEINS_VLC_FRONT:
        vlcMsg->setTransmissionModule(HEADLIGHT);
        LOG << nodeId << " Sending frame to the head light\n";
        break;
    case VEINS_VLC_BACK:
        vlcMsg->setTransmissionModule(TAILLIGHT);
        LOG << nodeId << " Sending frame to the tail light\n";
        break;
    case VEINS_VLC_FRONT | VEINS_VLC_BACK:
        vlcMsg->setTransmissionModule(BOTH_LIGHTS);
        LOG << nodeId << " Sending frame to both light\n";
        break;
    default:
        throw cRuntimeError("VeinsVLCRadioDriver received a frame to send that should not be handled by the VLC driver");
    }
    vlcMsg->setSentAt(simTime());
    vlcMsg->encapsulate(frame);
    sendDown(vlcMsg.release());
}

bool VeinsVLCRadioDriver::registerNode(int nodeId)
{
    this->nodeId = nodeId;
    std::vector<MacLayerVlc*> vlcMacs = FindModule<MacLayerVlc*>::findSubModules(getParentModule());
    if (!vlcMacs.size()) return false;
    for (auto vlcMac : vlcMacs) vlcMac->setMACAddress(VEH_ID_TO_MAC(nodeId));
    if (Mac1609_4* mac = FindModule<Mac1609_4*>::findSubModule(getParentModule())) {
        mac->setMACAddress(VEH_ID_TO_MAC(nodeId));
        return true;
    }
    else {
        return false;
    }
}

} // namespace plexe
