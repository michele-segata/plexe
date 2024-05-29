//
// Copyright (C) 2018-2024 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "SimpleIntersectionScenario.h"

using namespace veins;

namespace plexe {

Define_Module(SimpleIntersectionScenario);

void SimpleIntersectionScenario::initialize(int stage)
{

    SimpleScenario::initialize(stage);

    if (stage == 2) {
        checkDistance = new cMessage("checkDistance");
        scheduleAfter(SimTime(1), checkDistance);
    }
}

void SimpleIntersectionScenario::handleSelfMsg(cMessage* msg)
{
    if (msg == checkDistance) {
        // 500 meters is when the vehicle has crossed the intersection
        if (plexeTraciVehicle->getDistanceToRouteEnd() < 500) {
            endSimulation();
        }
        else {
            scheduleAfter(SimTime(1), checkDistance);
        }
    }
    else {
        SimpleScenario::handleSelfMsg(msg);
    }
}

void SimpleIntersectionScenario::finish()
{
    cancelAndDelete(checkDistance);
    checkDistance = nullptr;
}

} // namespace plexe
