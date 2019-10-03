//
// Copyright (C) 2018-2019 Michele Segata <segata@ccs-labs.org>
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

#include <plexe/plexe.h>

#include <veins/modules/mobility/traci/TraCIScenarioManager.h>
#include <veins/modules/mobility/traci/TraCICommandInterface.h>

#include <plexe/mobility/CommandInterface.h>

namespace plexe {

class PlexeManager : public cSimpleModule {
public:
    void initialize(int stage) override;

    /**
     * Return a weak pointer to the CommandInterface owned by this manager.
     */
    traci::CommandInterface* getCommandInterface()
    {
        return commandInterface.get();
    }

private:
    class DeferredCommandInterfaceInitializer : public cListener {
    public:
        DeferredCommandInterfaceInitializer(PlexeManager* owner)
            : owner(owner)
        {
        }

        void receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) override
        {
            ASSERT(signalID == veins::TraCIScenarioManager::traciInitializedSignal && b);

            owner->initializeCommandInterface();
        }

    private:
        PlexeManager* owner;
    };

    class PlexeTimestepTrigger : public cListener {
    public:
        PlexeTimestepTrigger(PlexeManager* owner)
            : owner(owner)
        {
        }

        void receiveSignal(cComponent*, simsignal_t signalID, const simtime_t&, cObject*) override
        {
            ASSERT(signalID == veins::TraCIScenarioManager::traciTimestepEndSignal);

            owner->getCommandInterface()->executePlexeTimestep();
        }

    private:
        PlexeManager* owner;
    };

    void initializeCommandInterface();

    DeferredCommandInterfaceInitializer deferredCommandInterfaceInitializer{this};
    PlexeTimestepTrigger plexeTimestepTrigger{this};
    std::unique_ptr<traci::CommandInterface> commandInterface;
};

} // namespace plexe
