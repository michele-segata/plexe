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

#include "SimplePlatooningBeaconing.h"

Define_Module(SimplePlatooningBeaconing)

    void SimplePlatooningBeaconing::initialize(int stage)
{
    BaseProtocol::initialize(stage);

    if (stage == 0) {
        // random start time
        SimTime beginTime = SimTime(uniform(0.001, beaconingInterval));
        if (beaconingInterval > 0) scheduleAt(simTime() + beaconingInterval + beginTime, sendBeacon);
    }
}

void SimplePlatooningBeaconing::handleSelfMsg(cMessage* msg)
{

    BaseProtocol::handleSelfMsg(msg);

    if (msg == sendBeacon) {
        sendPlatooningMessage(-1);
        scheduleAt(simTime() + beaconingInterval, sendBeacon);
    }
}

void SimplePlatooningBeaconing::messageReceived(PlatooningBeacon* pkt, UnicastMessage* unicast)
{
    // nothing to do for static beaconing
}

SimplePlatooningBeaconing::SimplePlatooningBeaconing()
{
}

SimplePlatooningBeaconing::~SimplePlatooningBeaconing()
{
}
