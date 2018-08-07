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

#ifndef RINGTRAFFICMANAGER_H_
#define RINGTRAFFICMANAGER_H_

#include "veins/modules/mobility/traci/TraCIBaseTrafficManager.h"
#include "veins/modules/application/platooning/utilities/DynamicPositionManager.h"

class RingTrafficManager : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage);
    virtual void scenarioLoaded();

    RingTrafficManager()
        : platoonSize(0)
        , nPlatoons(0)
        , injectedCars(0)
        , injectedPlatoons(0)
        , positions(DynamicPositionManager::getInstance())
    {
        platoonInsertDistance = 0;
        platoonInsertHeadway = 0;
        platoonInsertSpeed = 0;
        platoonLeaderHeadway = 0;
        nLanes = 0;
    }

protected:
    cPar* platoonSize;
    int nPlatoons;
    int injectedCars;
    int injectedPlatoons;
    DynamicPositionManager& positions;
    // number of lanes
    int nLanes;
    // insert distance
    double platoonInsertDistance;
    // insert headway
    double platoonInsertHeadway;
    // headway for leader vehicles
    double platoonLeaderHeadway;
    // sumo vehicle type of platooning cars
    std::string platooningVType;
    cPar* platoonInsertSpeed;

    typedef struct {
        int size;
        double speed;
        double length;
        double distanceToFront;
    } Platoon;
};

#endif
