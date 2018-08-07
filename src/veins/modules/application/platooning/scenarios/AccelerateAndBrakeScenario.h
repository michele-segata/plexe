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

#ifndef ACCELERATEANDBRAKESCENARIO_H_
#define ACCELERATEANDBRAKESCENARIO_H_

#include "veins/modules/application/platooning/scenarios/BaseScenario.h"

#include "veins/modules/application/platooning/apps/BaseApp.h"

class AccelerateAndBrakeScenario : public BaseScenario {

public:
    virtual void initialize(int stage);

protected:
    // leader average speed
    double leaderSpeed;
    // acceleration in m/s/s
    double acceleration;
    // braking intensity in m/s/s
    double brakingDeceleration;
    // message used to tell the car to start accelerating
    cMessage* startAccelerationMsg;
    // message used to tell the car to start braking
    cMessage* startBrakingMsg;
    // start accelerating time
    SimTime startAccelerating;
    // start braking time
    SimTime startBraking;
    // application layer, used to stop the simulation
    BaseApp* appl;

public:
    AccelerateAndBrakeScenario()
    {
        leaderSpeed = 0;
        acceleration = 0;
        brakingDeceleration = 0;
        startAccelerationMsg = nullptr;
        startBrakingMsg = nullptr;
        startAccelerating = SimTime(0);
        startBraking = SimTime(0);
        appl = 0;
    }
    virtual ~AccelerateAndBrakeScenario();

protected:
    virtual void handleSelfMsg(cMessage* msg);
};

#endif
