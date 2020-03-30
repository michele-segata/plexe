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

#ifndef HUMANINTERFERINGPROTOCOL_H_
#define HUMANINTERFERINGPROTOCOL_H_

#include "veins/base/modules/BaseApplLayer.h"

#include "plexe/messages/PlatooningBeacon_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe/utilities/BasePositionHelper.h"

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"

namespace plexe {

class HumanInterferingProtocol : public veins::BaseApplLayer {

private:
    // beacon interval
    SimTime beaconingInterval;
    // access category
    int priority;
    // packet size
    int packetSize;
    // transmit power in mW
    double txPower;
    // bit rate in bps
    int bitrate;

protected:
    // traci mobility. used for getting/setting info about the car
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;

    // pointer to the mac layer
    veins::Mac1609_4* mac;

    // messages for scheduleAt
    cMessage* sendBeacon;

    virtual void handleSelfMsg(cMessage* msg);

    virtual void handleLowerMsg(cMessage* msg);

    /**
     * Sends an interfering packet
     */
    void sendInterferingMessage();

public:
    // id for beacon message
    static const int INTERFERENCE_TYPE = 12349;

    HumanInterferingProtocol()
    {
        sendBeacon = nullptr;
    }
    virtual ~HumanInterferingProtocol();

    virtual void initialize(int stage);
};

} // namespace plexe

#endif
