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

#include "veins/modules/application/platooning/scenarios/BrakingScenario.h"

using namespace Veins;

Define_Module(BrakingScenario);

void BrakingScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 0)
        // get pointer to application
        appl = FindModule<BaseApp*>::findSubModule(getParentModule());

    if (stage == 1) {
        // get braking deceleration
        brakingDeceleration = par("brakingDeceleration").doubleValue();
        // average speed
        leaderSpeed = par("leaderSpeed").doubleValue() / 3.6;
        // start braking time
        startBraking = SimTime(par("startBraking").doubleValue());

        if (positionHelper->getId() < positionHelper->getLanesCount()) {
            // setup braking message, only if i'm part of the first leaders
            changeSpeed = new cMessage("changeSpeed");
            if (simTime() > startBraking) {
                startBraking = simTime();
                scheduleAt(simTime(), changeSpeed);
            }
            else {
                scheduleAt(startBraking, changeSpeed);
            }
            // set base cruising speed
            traciVehicle->setCruiseControlDesiredSpeed(leaderSpeed);
        }
        else {
            // let the follower set a higher desired speed to stay connected
            // to the leader when it is accelerating
            traciVehicle->setCruiseControlDesiredSpeed(leaderSpeed + 10);
        }
    }
}

BrakingScenario::~BrakingScenario()
{
    cancelAndDelete(changeSpeed);
    changeSpeed = nullptr;
}

void BrakingScenario::handleSelfMsg(cMessage* msg)
{
    BaseScenario::handleSelfMsg(msg);
    if (msg == changeSpeed) traciVehicle->setFixedAcceleration(1, -brakingDeceleration);
}
