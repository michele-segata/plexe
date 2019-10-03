//
// Copyright (C) 2014-2019 Michele Segata <segata@ccs-labs.org>
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

#include "PlatoonsTrafficManager.h"

namespace plexe {

Define_Module(PlatoonsTrafficManager);

void PlatoonsTrafficManager::initialize(int stage)
{

    TraCIBaseTrafficManager::initialize(stage);

    if (stage == 0) {

        nCars = par("nCars");
        platoonSize = par("platoonSize");
        nLanes = par("nLanes");
        platoonInsertTime = SimTime(par("platoonInsertTime").doubleValue());
        platoonInsertSpeed = par("platoonInsertSpeed").doubleValue();
        platoonInsertDistance = par("platoonInsertDistance").doubleValue();
        platoonInsertHeadway = par("platoonInsertHeadway").doubleValue();
        platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();
        platooningVType = par("platooningVType").stdstringValue();
        insertPlatoonMessage = new cMessage("");
        scheduleAt(platoonInsertTime, insertPlatoonMessage);
    }
}

void PlatoonsTrafficManager::scenarioLoaded()
{
    automated.id = findVehicleTypeIndex(platooningVType);
    automated.lane = -1;
    automated.position = 0;
    automated.speed = platoonInsertSpeed / 3.6;
}

void PlatoonsTrafficManager::handleSelfMsg(cMessage* msg)
{

    TraCIBaseTrafficManager::handleSelfMsg(msg);

    if (msg == insertPlatoonMessage) {
        insertPlatoons();
    }
}

void PlatoonsTrafficManager::insertPlatoons()
{

    // compute intervehicle distance
    double distance = platoonInsertSpeed / 3.6 * platoonInsertHeadway + platoonInsertDistance;
    // total number of platoons per lane
    int nPlatoons = nCars / platoonSize / nLanes;
    // length of 1 platoon
    double platoonLength = platoonSize * 4 + (platoonSize - 1) * distance;
    // inter-platoon distance
    double platoonDistance = platoonInsertSpeed / 3.6 * platoonLeaderHeadway;
    // total length for one lane
    double totalLength = nPlatoons * platoonLength + (nPlatoons - 1) * platoonDistance;

    // for each lane, we create an offset to have misaligned platoons
    double* laneOffset = new double[nLanes];
    for (int l = 0; l < nLanes; l++) laneOffset[l] = uniform(0, 20);

    double currentRoadPosition = totalLength;
    int currentVehiclePosition = 0;
    int currentVehicleId = 0;
    int basePlatoonId = 0;
    for (int i = 0; i < nCars / nLanes; i++) {
        for (int l = 0; l < nLanes; l++) {
            automated.position = currentRoadPosition + laneOffset[l];
            automated.lane = l;
            addVehicleToQueue(0, automated);
            positions.addVehicleToPlatoon(currentVehicleId, currentVehiclePosition, basePlatoonId + l);
            currentVehicleId++;
            if (currentVehiclePosition == 0) {
                PlatoonInfo info;
                info.speed = automated.speed;
                info.lane = automated.lane;
                positions.setPlatoonInformation(basePlatoonId + l, info);
            }
        }
        currentVehiclePosition++;
        if (currentVehiclePosition == platoonSize) {
            currentVehiclePosition = 0;
            // add inter platoon gap
            currentRoadPosition -= (platoonDistance + 4);
            basePlatoonId += nLanes;
        }
        else {
            // add intra platoon gap
            currentRoadPosition -= (4 + distance);
        }
    }

    delete[] laneOffset;
}

PlatoonsTrafficManager::~PlatoonsTrafficManager()
{
    cancelAndDelete(insertPlatoonMessage);
    insertPlatoonMessage = nullptr;
}

} // namespace plexe
