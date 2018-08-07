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

#include "veins/modules/application/platooning/traffic/RingTrafficManager.h"

Define_Module(RingTrafficManager);

void RingTrafficManager::initialize(int stage)
{

    TraCIBaseTrafficManager::initialize(stage);

    platoonSize = &par("platoonSize");
    nPlatoons = par("nPlatoons").longValue();
    nLanes = par("nLanes").longValue();
    platoonInsertSpeed = &par("platoonInsertSpeed");
    platoonInsertDistance = par("platoonInsertDistance").doubleValue();
    platoonInsertHeadway = par("platoonInsertHeadway").doubleValue();
    platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();
    platooningVType = par("platooningVType").stdstringValue();
}

void RingTrafficManager::scenarioLoaded()
{

    int vehTypeId = findVehicleTypeIndex("vtypeauto");
    struct Vehicle automated;

    // map from lane index to a vector with all platoon sizes
    std::map<int, std::vector<Platoon>> platoons;
    // total vehicle length for each lane
    std::vector<double> lengths;

    for (int l = 0; l < nLanes; l++) {
        platoons[l] = std::vector<Platoon>();
        lengths.push_back(0);
    }

    // pre-compute the platoons to be inserted
    // TODO: the ring is composed by two half-rings. position/route of vehicles
    // must depend on the length of the ring
    // TODO: add human driven vehicles as well
    int l = 0;
    for (int p = 0; p < nPlatoons; p++) {
        Platoon platoon;
        // get the number of cars in this platoon
        platoon.size = platoonSize->longValue();
        // get the speed of this platoon
        platoon.speed = platoonInsertSpeed->doubleValue() / 3.6;
        // compute the distance of this platoon
        platoon.distanceToFront = platoonLeaderHeadway * platoon.speed;
        // compute the length of the platoon. assume a hardcoded vehicle length value of 4
        platoon.length = platoon.size * 4 + (platoon.size - 1) * (platoonInsertDistance + platoonInsertHeadway * platoon.speed);
        // add the length of this platoon to the platoons of this lane
        lengths[l] += platoon.length + platoon.distanceToFront;
        platoons.find(l)->second.push_back(platoon);
        // loop through all lanes
        l = (l + 1) % nLanes;
    }

    // finally inject vehicles
    double totalLength;
    for (l = 0; l < nLanes; l++) {
        totalLength = lengths[l];
        std::vector<Platoon> ps = platoons.find(l)->second;
        for (int p = 0; p < ps.size(); p++) {
            automated.id = vehTypeId;
            automated.speed = ps[p].speed;
            totalLength -= platoonLeaderHeadway * ps[p].speed;
            for (int v = 0; v < ps[p].size; v++) {
                automated.position = totalLength;
                automated.lane = l;
                addVehicleToQueue(0, automated);
                positions.addVehicleToPlatoon(injectedCars, v, injectedPlatoons);
                injectedCars++;
                if (v < ps[p].size - 1)
                    totalLength -= (4 + platoonInsertDistance + platoonInsertHeadway * automated.speed);
                else
                    totalLength -= 4;
            }
            injectedPlatoons++;
        }
    }
}
