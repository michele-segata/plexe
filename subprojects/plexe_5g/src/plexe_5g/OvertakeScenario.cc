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

#include "OvertakeScenario.h"

namespace plexe {

Define_Module(OvertakeScenario);

void OvertakeScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 2) {
        prepareManeuverCars();
    }
}

void OvertakeScenario::prepareManeuverCars()
{
    if (positionHelper->isLeader() && positionHelper->getPlatoonId() == PLATOON_ID_A) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed());
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(0);
    }
    else if (positionHelper->isLeader() && positionHelper->getPlatoonId() == PLATOON_ID_B) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed());
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(0);
    }
    else if (positionHelper->isLeader() && positionHelper->getPlatoonId() == PLATOON_ID_C) {
        plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed());
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(0);
    }
    else {
        // these are the followers which are already in the platoon
        plexeTraciVehicle->setCruiseControlDesiredSpeed(130.0 / 3.6);
        plexeTraciVehicle->setActiveController(positionHelper->getController());
        plexeTraciVehicle->setFixedLane(0);
    }
}

OvertakeScenario::~OvertakeScenario()
{
}

} // namespace plexe
