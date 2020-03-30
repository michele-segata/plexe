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

#include "SimplePlatooningBeaconing.h"

namespace plexe {

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

SimplePlatooningBeaconing::SimplePlatooningBeaconing()
{
}

SimplePlatooningBeaconing::~SimplePlatooningBeaconing()
{
}

} // namespace plexe
