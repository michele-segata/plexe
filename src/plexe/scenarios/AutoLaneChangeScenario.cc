//
// Copyright (C) 2012-2019 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/scenarios/AutoLaneChangeScenario.h"

using namespace veins;

namespace plexe {

Define_Module(AutoLaneChangeScenario);

void AutoLaneChangeScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 0)
        // get pointer to application
        appl = FindModule<BaseApp*>::findSubModule(getParentModule());

    if (stage == 1) {
        platooningVType = par("platooningVType").stdstringValue();

        plexeTraciVehicle->setFixedLane(traciVehicle->getLaneIndex(), false);
        traciVehicle->setSpeedMode(0);
        if (positionHelper->isLeader()) {
            for (int i = 1; i < positionHelper->getPlatoonSize(); i++) {
                std::stringstream ss;
                ss << platooningVType << "." << positionHelper->getMemberId(i);
                plexeTraciVehicle->addPlatoonMember(ss.str(), i);
            }
            plexeTraciVehicle->enableAutoLaneChanging(true);
            plexeTraciVehicle->setCruiseControlDesiredSpeed(mobility->getSpeed());
        }
        else {
            std::stringstream ssl, ss;
            ssl << platooningVType << "." << positionHelper->getLeaderId();
            ss << platooningVType << "." << positionHelper->getFrontId();
            plexeTraciVehicle->enableAutoFeed(true, ssl.str(), ss.str());
            plexeTraciVehicle->setCruiseControlDesiredSpeed(mobility->getSpeed() + 10);
        }
    }
}

} // namespace plexe
