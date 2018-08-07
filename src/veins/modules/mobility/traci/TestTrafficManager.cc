//
// Copyright (C) 2013-2018 Michele Segata <segata@ccs-labs.org>, Stefan Joerer <joerer@ccs-labs.org>
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

#include "veins/modules/mobility/traci/TestTrafficManager.h"

Define_Module(TestTrafficManager);

void TestTrafficManager::initialize(int stage)
{

    TraCIBaseTrafficManager::initialize(stage);

    if (stage == 0) {

        generateVehicle = new cMessage("generateVehicle");

        scheduleAt(simTime() + SimTime(0.1), generateVehicle);
    }
}

void TestTrafficManager::handleSelfMsg(cMessage* msg)
{

    TraCIBaseTrafficManager::handleSelfMsg(msg);

    if (msg == generateVehicle) {
        insertNewVehicle();
    }
}

void TestTrafficManager::insertNewVehicle()
{
    int vehTypeId = 0;
    int routeId = 0;
    int i;
    struct Vehicle v;
    v.id = vehTypeId;
    v.lane = -1;
    v.position = 0;
    v.speed = -1;
    // insert nCars of the same type and with the same route, for testing purposes
    for (i = 0; i < par("nCars").longValue(); i++) {
        addVehicleToQueue(routeId, v);
    }
}

void TestTrafficManager::finish()
{
    TraCIBaseTrafficManager::finish();
    if (generateVehicle) {
        cancelAndDelete(generateVehicle);
        generateVehicle = 0;
    }
}
