//
// Copyright (C) 2014-2025 Michele Segata <segata@ccs-labs.org>
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

#ifndef PLATOONSTRAFFICMANAGER_H_
#define PLATOONSTRAFFICMANAGER_H_

#include "plexe/mobility/TraCIBaseTrafficManager.h"

namespace plexe {

class PlatoonsTrafficManager : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage);

    PlatoonsTrafficManager()
    {
        insertPlatoonMessage = nullptr;
        platoonInsertDistance = 0;
        platoonInsertHeadway = 0;
        platoonInsertSpeed = 0;
        platoonInsertTime = SimTime(0);
        platoonLeaderHeadway = 0;
        platoonAdditionalDistance = 0;
        platoonSize = 0;
        nCars = 0;
        nLanes = 0;
    }
    virtual ~PlatoonsTrafficManager();

protected:
    // this is used to start traffic generation
    cMessage* insertPlatoonMessage;

    void insertPlatoons();

    virtual void handleSelfMsg(cMessage* msg);

    SimTime platoonInsertTime;
    double platoonInsertSpeed;
    // vehicles to be inserted
    struct Vehicle automated;

    // total number of vehicles to be injected
    int nCars;
    // vehicles per platoon
    int platoonSize;
    // number of lanes
    int nLanes;
    // insert distance
    double platoonInsertDistance;
    // insert headway
    double platoonInsertHeadway;
    // headway for leader vehicles
    double platoonLeaderHeadway;
    // additional distance between consecutive platoons
    double platoonAdditionalDistance;
    // sumo vehicle type of platooning cars
    std::string platooningVType;

    virtual void scenarioLoaded();
};

} // namespace plexe

#endif
