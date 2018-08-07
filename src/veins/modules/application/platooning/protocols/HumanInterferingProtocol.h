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

#ifndef HUMANINTERFERINGPROTOCOL_H_
#define HUMANINTERFERINGPROTOCOL_H_

#include "veins/base/modules/BaseApplLayer.h"

#include "veins/modules/application/platooning/UnicastProtocol.h"
#include "veins/modules/application/platooning/messages/PlatooningBeacon_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"

class HumanInterferingProtocol : public Veins::BaseApplLayer {

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
    Veins::TraCIMobility* mobility;
    Veins::TraCICommandInterface* traci;
    Veins::TraCICommandInterface::Vehicle* traciVehicle;

    // pointer to the mac layer
    Veins::Mac1609_4* mac;

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

#endif
