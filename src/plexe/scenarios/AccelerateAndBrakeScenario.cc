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

#include "plexe/scenarios/AccelerateAndBrakeScenario.h"

using namespace veins;

namespace plexe {

Define_Module(AccelerateAndBrakeScenario);

void AccelerateAndBrakeScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 0)
        // get pointer to application
        appl = FindModule<BaseApp*>::findSubModule(getParentModule());

    if (stage == 1) {
        // get acceleration
        acceleration = par("acceleration").doubleValue();
        // get braking deceleration
        brakingDeceleration = par("brakingDeceleration").doubleValue();
        // average speed
        leaderSpeed = par("leaderSpeed").doubleValue() / 3.6;
        // start accelerating time
        startAccelerating = SimTime(par("startAccelerating").doubleValue());
        // start braking time
        startBraking = SimTime(par("startBraking").doubleValue());

        // messages to schedule actions
        startAccelerationMsg = new cMessage("startAccelerationMsg");
        startBrakingMsg = new cMessage("startBrakingMsg");

        // enable ACC
        plexeTraciVehicle->setActiveController(ACC);
        // let the vehicle start from standstill
        plexeTraciVehicle->setFixedAcceleration(1, -8);

        // schedule messages
        scheduleAt(startAccelerating, startAccelerationMsg);
        scheduleAt(startBraking, startBrakingMsg);
    }
}

AccelerateAndBrakeScenario::~AccelerateAndBrakeScenario()
{
    cancelAndDelete(startAccelerationMsg);
    startAccelerationMsg = nullptr;
    cancelAndDelete(startBrakingMsg);
    startBrakingMsg = nullptr;
}

void AccelerateAndBrakeScenario::handleSelfMsg(cMessage* msg)
{
    BaseScenario::handleSelfMsg(msg);
    if (msg == startAccelerationMsg) plexeTraciVehicle->setFixedAcceleration(1, acceleration);
    if (msg == startBrakingMsg) plexeTraciVehicle->setFixedAcceleration(1, -brakingDeceleration);
}

} // namespace plexe
