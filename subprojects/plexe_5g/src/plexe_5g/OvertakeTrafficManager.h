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

#pragma once

#include "plexe/mobility/TraCIBaseTrafficManager.h"

namespace plexe {

class OvertakeTrafficManager : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage);

    OvertakeTrafficManager()
    {
        insertPlatoonMessage = nullptr;
        platoonInsertDistance = 0;
        platoonInsertHeadway = 0;
        platoonInsertTime = SimTime(0);
        platoonLeaderHeadway = 0;
        platoonAdditionalDistance = 0;

        platoonASpeed = 0;
        platoonBSpeed = 0;
        platoonSizeA = 0;
        platoonSizeB = 0;
        platoonSizeC = 0;
        initialPositionDeltaA = 0;
        initialPositionDeltaB = 0;
        initialPositionDeltaC = 0;
    }
    virtual ~OvertakeTrafficManager();

protected:
    // this is used to start traffic generation
    cMessage* insertPlatoonMessage;

    void insertPlatoons();

    virtual void handleSelfMsg(cMessage* msg);

    SimTime platoonInsertTime;
    // vehicles to be inserted
    struct Vehicle automated;

    // vehicles per platoon
    int platoonSizeA;
    // vehicles per platoon
    int platoonSizeB;
    // vehicles per platoon
    int platoonSizeC;

    double platoonASpeed;
    double platoonBSpeed;
    double platoonCSpeed;

    int initialPositionDeltaA;
    int initialPositionDeltaB;
    int initialPositionDeltaC;
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
