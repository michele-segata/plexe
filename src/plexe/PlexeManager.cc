//
// Copyright (c) 2019 Max Schettler <schettler@ccs-labs.org>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#include "PlexeManager.h"

#include "plexe/mobility/CommandInterface.h"

namespace plexe {

Define_Module(PlexeManager);

void PlexeManager::initialize(int stage)
{
    const auto scenarioManager = veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);

    scenarioManager->subscribe(veins::TraCIScenarioManager::traciTimestepEndSignal, &plexeTimestepTrigger);

    if (scenarioManager->isUsable()) {
        initializeCommandInterface();
    }
    else {
        scenarioManager->subscribe(veins::TraCIScenarioManager::traciInitializedSignal, &deferredCommandInterfaceInitializer);
    }
}

void PlexeManager::initializeCommandInterface()
{

    const auto scenarioManager = veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);
    commandInterface.reset(new traci::CommandInterface(this, scenarioManager->getCommandInterface(), scenarioManager->getConnection()));
}

} // namespace plexe
