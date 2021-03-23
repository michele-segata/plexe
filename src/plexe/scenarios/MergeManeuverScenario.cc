//
// Copyright (C) 2012-2021 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/scenarios/MergeManeuverScenario.h"

namespace plexe {

Define_Module(MergeManeuverScenario);

void MergeManeuverScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 1) {
        app = FindModule<GeneralPlatooningApp*>::findSubModule(getParentModule());
        prepareManeuverCars(0);
    }
}

void MergeManeuverScenario::prepareManeuverCars(int platoonLane)
{
    if (positionHelper->getId() == 0) {
        // this is the leader of the platoon ahead
        plexeTraciVehicle->setCruiseControlDesiredSpeed(100.0 / 3.6);
        plexeTraciVehicle->setActiveController(ACC);
        plexeTraciVehicle->setFixedLane(platoonLane);
        app->setPlatoonRole(PlatoonRole::LEADER);
    }
    else if (!positionHelper->isLeader()) {
        // these are the followers which are already in the platoon
        plexeTraciVehicle->setCruiseControlDesiredSpeed(130.0 / 3.6);
        plexeTraciVehicle->setActiveController(CACC);
        plexeTraciVehicle->setFixedLane(platoonLane);
        app->setPlatoonRole(PlatoonRole::FOLLOWER);
    }
    else {
        // this is the leader which will merge
        plexeTraciVehicle->setCruiseControlDesiredSpeed(100 / 3.6);
        plexeTraciVehicle->setFixedLane(platoonLane);
        plexeTraciVehicle->setActiveController(ACC);
        app->setPlatoonRole(PlatoonRole::LEADER);

        // after 30 seconds of simulation, start the maneuver
        startManeuver = new cMessage();
        scheduleAt(simTime() + SimTime(10), startManeuver);
    }
}

MergeManeuverScenario::~MergeManeuverScenario()
{
    cancelAndDelete(startManeuver);
    startManeuver = nullptr;
}

void MergeManeuverScenario::handleSelfMsg(cMessage* msg)
{

    // this takes car of feeding data into CACC and reschedule the self message
    BaseScenario::handleSelfMsg(msg);

    LOG << "Starting the merge maneuver\n";
    if (msg == startManeuver) app->startMergeManeuver(0, 0, -1);
}

} // namespace plexe
