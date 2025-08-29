//
// Copyright (C) 2020-2025 Michele Segata <segata@ccs-labs.org>
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

#include "CooperisRadioDriver.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/base/utils/FindModule.h"
#include "cooperis/mac/MacLayerRis.h"
#include "plexe/messages/PlatooningBeacon_m.h"

#define VEH_ID_TO_MAC(x) (x + 1)
#define MAC_TO_VEH_ID(x) (x - 1)

using namespace veins;

namespace plexe {

Define_Module(plexe::CooperisRadioDriver);

void CooperisRadioDriver::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);
    if (frame->getRecipientAddress() != veins::LAddress::L2BROADCAST()) frame->setRecipientAddress(MAC_TO_VEH_ID(frame->getRecipientAddress()));
    sendUp(frame);
}

void CooperisRadioDriver::handleUpperMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);
    if (frame->getRecipientAddress() != veins::LAddress::L2BROADCAST()) frame->setRecipientAddress(VEH_ID_TO_MAC(frame->getRecipientAddress()));
    sendDown(frame);
}

bool CooperisRadioDriver::registerNode(int nodeId)
{
    if (MacLayerRis* mac = FindModule<MacLayerRis*>::findSubModule(getParentModule())) {
        mac->setMACAddress(VEH_ID_TO_MAC(nodeId));
        return true;
    }
    else {
        return false;
    }
}

} // namespace plexe
