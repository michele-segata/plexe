//
// Copyright (C) 2013-2019 Michele Segata <segata@ccs-labs.org>, Stefan Joerer <joerer@ccs-labs.org>
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

#include "plexe/mobility/TestTrafficManager.h"

namespace plexe {

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
    int nCars = par("nCars");
    for (i = 0; i < nCars; i++) {
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

} // namespace plexe
