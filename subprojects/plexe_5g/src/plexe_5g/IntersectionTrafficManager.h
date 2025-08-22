//
// Copyright (C) 2014-2022 Michele Segata <segata@ccs-labs.org>
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

#pragma once

#include "plexe/mobility/TraCIBaseTrafficManager.h"

namespace plexe {

class IntersectionTrafficManager : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage);

    IntersectionTrafficManager()
    {
        insertPlatoonMessage = nullptr;
        platoonInsertTime = SimTime(0);
        platoonInsertSpeed = 0;

        platoonSizeA = 0;
        platoonSizeB = 0;
        platoonSizeC = 0;
        initialPositionDeltaA = 0;
        initialPositionDeltaB = 0;
        initialPositionDeltaC = 0;
        routeLeftRight = "";
        routeBottomRight = "";

        platoonInsertDistance = 0;
        platoonInsertHeadway = 0;
        platoonLeaderHeadway = 0;
    }
    virtual ~IntersectionTrafficManager();

protected:
    // this is used to start traffic generation
    cMessage* insertPlatoonMessage;

    void insertPlatoons();

    virtual void handleSelfMsg(cMessage* msg);

    SimTime platoonInsertTime;
    double platoonInsertSpeed;
    // vehicles to be inserted
    struct Vehicle automated;
    struct Vehicle shockwave;

    // platoon size
    int platoonSizeA;
    int platoonSizeB;
    int platoonSizeC;
    // randomized initial position
    volatile double initialPositionDeltaA;
    volatile double initialPositionDeltaB;
    volatile double initialPositionDeltaC;
    // routes
    std::string routeLeftRight;
    std::string routeBottomRight;

    // insert distance
    double platoonInsertDistance;
    // insert headway
    double platoonInsertHeadway;
    // inter platoon headway
    double platoonLeaderHeadway;
    // sumo vehicle type of platooning cars
    std::string platooningVType;
    std::string shockwaveVType;
    bool injectShockwaveCars = false;


    virtual void scenarioLoaded();
};

} // namespace plexe
