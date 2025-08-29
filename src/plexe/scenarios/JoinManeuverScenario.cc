//
// Copyright (C) 2012-2025 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/scenarios/JoinManeuverScenario.h"
#include "plexe/mobility/TraCIBaseTrafficManager.h"

namespace plexe {

Define_Module(JoinManeuverScenario);

void JoinManeuverScenario::initialize(int stage)
{

    ManeuverScenario::initialize(stage);

    if (stage == 2) {
        prepareManeuverCars(0);
    }
}

void JoinManeuverScenario::setupFormation()
{
    std::vector<int> formation;
    for (int i = 0; i < 4; i++) formation.push_back(i);
    positionHelper->setPlatoonFormation(formation);
}

void JoinManeuverScenario::prepareManeuverCars(int platoonLane)
{

    switch (positionHelper->getId()) {

    case 0: {
        // this is the leader
        plexeTraciVehicle->setCruiseControlDesiredSpeed(100.0 / 3.6);
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(platoonLane);
        app->setPlatoonRole(PlatoonRole::LEADER);
        break;
    }

    case 1:
    case 2:
    case 3: {
        // these are the followers which are already in the platoon
        plexeTraciVehicle->setCruiseControlDesiredSpeed(130.0 / 3.6);
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(platoonLane);
        app->setPlatoonRole(PlatoonRole::FOLLOWER);
        break;
    }

    case 4: {
        // this is the car which will join
        plexeTraciVehicle->setCruiseControlDesiredSpeed(100 / 3.6);
        plexeTraciVehicle->setFixedLane(2);
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        app->setPlatoonRole(PlatoonRole::NONE);

        // after 30 seconds of simulation, start the maneuver
        startManeuver = new cMessage();
        scheduleAt(simTime() + SimTime(10), startManeuver);
        break;
    }
    }
}

void JoinManeuverScenario::handleSelfMsg(cMessage* msg)
{

    // this takes car of feeding data into CACC and reschedule the self message
    ManeuverScenario::handleSelfMsg(msg);

    if (msg == startManeuver) app->startJoinManeuver(0, 0, -1);
}

} // namespace plexe
