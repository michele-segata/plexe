//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
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

#include "veins/modules/application/platooning/scenarios/AutoLaneChangeScenario.h"

using namespace Veins;

Define_Module(AutoLaneChangeScenario);

void AutoLaneChangeScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 0)
        // get pointer to application
        appl = FindModule<BaseApp*>::findSubModule(getParentModule());

    if (stage == 1) {
        platooningVType = par("platooningVType").stdstringValue();

        traciVehicle->setFixedLane(traciVehicle->getLaneIndex(), false);
        traciVehicle->setSpeedMode(0);
        if (positionHelper->isLeader()) {
            for (int i = 1; i < positionHelper->getPlatoonSize(); i++) {
                std::stringstream ss;
                ss << platooningVType << "." << positionHelper->getMemberId(i);
                traciVehicle->addPlatoonMember(ss.str(), i);
            }
            traciVehicle->enableAutoLaneChanging(true);
            traciVehicle->setCruiseControlDesiredSpeed(mobility->getSpeed());
        }
        else {
            std::stringstream ssl, ss;
            ssl << platooningVType << "." << positionHelper->getLeaderId();
            ss << platooningVType << "." << positionHelper->getFrontId();
            traciVehicle->enableAutoFeed(true, ssl.str(), ss.str());
            traciVehicle->setCruiseControlDesiredSpeed(mobility->getSpeed() + 10);
        }
    }
}
