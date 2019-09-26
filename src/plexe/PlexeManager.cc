//
// Copyright (C) 2019 Michele Segata <segata@ccs-labs.org>
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

#include "PlexeManager.h"

#include "plexe/mobility/CommandInterface.h"

namespace plexe {

Define_Module(PlexeManager);

void PlexeManager::initialize(int stage)
{
    const auto scenarioManager = veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);

    if (scenarioManager->isUsable()) {
        initializeCommandInterface();
    }
    else {
        auto init = [this](veins::SignalPayload<bool>) { initializeCommandInterface(); };
        signalManager.subscribeCallback(scenarioManager, veins::TraCIScenarioManager::traciInitializedSignal, init);
    }
}

void PlexeManager::initializeCommandInterface()
{
    const auto scenarioManager = veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);
    commandInterface.reset(new traci::CommandInterface(this, scenarioManager->getCommandInterface(), scenarioManager->getConnection()));

    auto timestep = [this](veins::SignalPayload<simtime_t const&>) { commandInterface->executePlexeTimestep(); };
    signalManager.subscribeCallback(scenarioManager, veins::TraCIScenarioManager::traciTimestepEndSignal, timestep);
}

} // namespace plexe
