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

#pragma once

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe/CC_Const.h"
#include "plexe/mobility/CommandInterface.h"

namespace plexe {

class HumanApp : public cSimpleModule {

public:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override {return 2;}

protected:

    int myId = -1;

    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    std::unique_ptr<traci::CommandInterface::Vehicle> plexeTraciVehicle;

    double enableLaneChangeAfter = 5;
    bool enableLogging = true;
    double loggingInterval = 0.1;
    /**
     * Log data about vehicle
     */
    virtual void logVehicleData(bool crashed = false);

    // id of the vehicle
    simsignal_t nodeIdSignal;
    simsignal_t speedSignal;
    simsignal_t posxSignal;
    simsignal_t posySignal;

    // messages for scheduleAt
    cMessage* recordData = nullptr;
    // message to re-activate lane changes
    cMessage* enableLaneChanging = nullptr;

public:
    HumanApp()
    {
    }
    virtual ~HumanApp();

protected:
    virtual void handleMessage(cMessage* msg) override;
};

} // namespace plexe
