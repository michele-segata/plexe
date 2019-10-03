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

#include "plexe/scenarios/SinusoidalScenario.h"

namespace plexe {

Define_Module(SinusoidalScenario);

void SinusoidalScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 1) {
        // get the oscillation frequency of the leader as parameter
        leaderOscillationFrequency = par("leaderOscillationFrequency").doubleValue();
        // oscillation amplitude
        oscillationAmplitude = par("oscillationAmplitude").doubleValue() / 3.6;
        // average speed
        leaderSpeed = par("leaderSpeed").doubleValue() / 3.6;
        // number of lanes
        nLanes = par("nLanes").intValue();
        // start oscillation time
        startOscillating = SimTime(par("startOscillating").doubleValue());

        if (positionHelper->getId() < nLanes) {
            // setup oscillation message, only if i'm part of the first leaders
            changeSpeed = new cMessage("changeSpeed");
            if (simTime() > startOscillating) {
                startOscillating = simTime();
                scheduleAt(simTime(), changeSpeed);
            }
            else {
                scheduleAt(startOscillating, changeSpeed);
            }
            // set base cruising speed
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed);
        }
        else {
            // let the follower set a higher desired speed to stay connected
            // to the leader when it is accelerating
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed + 2 * oscillationAmplitude);
        }
    }
}

SinusoidalScenario::~SinusoidalScenario()
{
    cancelAndDelete(changeSpeed);
    changeSpeed = nullptr;
}

void SinusoidalScenario::handleSelfMsg(cMessage* msg)
{
    BaseScenario::handleSelfMsg(msg);
    if (msg == changeSpeed) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed + oscillationAmplitude * sin(2 * M_PI * (simTime() - startOscillating).dbl() * leaderOscillationFrequency));
        scheduleAt(simTime() + SimTime(0.1), changeSpeed);
    }
}

} // namespace plexe
