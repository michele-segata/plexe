//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/apps/HumanApp.h"
#include "plexe/PlexeManager.h"
#include "plexe/utilities/BasePositionHelper.h"

using namespace veins;

namespace plexe {

Define_Module(HumanApp);

void HumanApp::initialize(int stage)
{

    if (stage == 0) {

        // registering signals emitted to record nodeId, speed...
        // id of the vehicle
        nodeIdSignal = registerSignal("nocommNodeId");
        // speed and position
        speedSignal = registerSignal("nocommSpeed");
        posxSignal = registerSignal("nocommPosx");
        posySignal = registerSignal("nocommPosy");

        enableLaneChangeAfter = par("enableLaneChangeAfter");
        enableLogging = par("enableLogging").boolValue();
        loggingInterval = par("loggingInterval").doubleValue();

    }

    if (stage == 1) {
        mobility = veins::TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        auto plexe = FindModule<PlexeManager*>::findGlobalModule();
        ASSERT(plexe);
        plexeTraci = plexe->getCommandInterface();
        plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, mobility->getExternalId()));

        plexeTraciVehicle->setLaneChangeMode(FIX_LC);

        myId = BasePositionHelper::getIdFromExternalId(mobility->getExternalId());

        enableLaneChanging = new cMessage("enableLaneChanging");
        scheduleAfter(enableLaneChangeAfter, enableLaneChanging);

        if (enableLogging)
            recordData = new cMessage("recordData");
        // init statistics collection. round to 0.1 seconds
        SimTime rounded = SimTime(floor(simTime().dbl() * 1000 + 100), SIMTIME_MS);
        scheduleAt(rounded, recordData);
    }
}

HumanApp::~HumanApp()
{
    cancelAndDelete(recordData);
    recordData = nullptr;
    cancelAndDelete(enableLaneChanging);
    enableLaneChanging = nullptr;
}

void HumanApp::logVehicleData(bool crashed)
{
    // emit signals!
    emit(nodeIdSignal, myId);
    emit(speedSignal, mobility->getSpeed());
    Coord pos = mobility->getPositionAt(simTime());
    emit(posxSignal, pos.x);
    emit(posySignal, pos.y);
}

void HumanApp::handleMessage(cMessage* msg)
{
    if (msg == enableLaneChanging) {
        plexeTraciVehicle->setLaneChangeMode(DEFAULT_LC);
        return;
    }
    if (msg == recordData) {
        logVehicleData(false);
        // re-schedule next event
        scheduleAfter(loggingInterval, recordData);
        return;
    }
}

} // namespace plexe
