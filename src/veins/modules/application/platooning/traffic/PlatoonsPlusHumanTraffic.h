//
// Copyright (C) 2014-2018 Michele Segata <segata@ccs-labs.org>
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

#ifndef PLATOONSPLUSHUMANTRAFFIC_H_
#define PLATOONSPLUSHUMANTRAFFIC_H_

#include <veins/modules/mobility/traci/TraCIBaseTrafficManager.h>

class PlatoonsPlusHumanTraffic : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage);

    PlatoonsPlusHumanTraffic()
    {
        insertPlatoonMessage = nullptr;
        platoonInsertDistance = 0;
        platoonInsertHeadway = 0;
        platoonInsertSpeed = 0;
        platoonInsertTime = SimTime(0);
        platoonLeaderHeadway = 0;
        platoonSize = 0;
        nCars = 0;
        nLanes = 0;
        humanCars = 0;
        humanLanes = 0;
    }
    virtual ~PlatoonsPlusHumanTraffic();

protected:
    // this is used to start traffic generation
    cMessage* insertPlatoonMessage;

    void insertPlatoons();
    void insertHumans();

    virtual void handleSelfMsg(cMessage* msg);

    SimTime platoonInsertTime;
    double platoonInsertSpeed;
    // vehicles to be inserted
    struct Vehicle automated;
    struct Vehicle human;

    // total number of vehicles to be injected
    int nCars;
    // vehicles per platoon
    int platoonSize;
    // number of lanes
    int nLanes;
    // number of human vehicles
    int humanCars;
    // number of lanes for human vehicles
    int humanLanes;
    // insert distance
    double platoonInsertDistance;
    // insert headway
    double platoonInsertHeadway;
    // headway for leader vehicles
    double platoonLeaderHeadway;
    // sumo vehicle type of platooning cars
    std::string platooningVType;
    // sumo vehicle type of human driven cars
    std::string humanVType;

    virtual void scenarioLoaded();
};

#endif
