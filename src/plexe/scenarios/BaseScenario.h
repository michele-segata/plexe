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

#ifndef BASESCENARIO_H_
#define BASESCENARIO_H_

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe/utilities/BasePositionHelper.h"
#include "plexe/mobility/CommandInterface.h"

namespace plexe {

class BaseScenario : public veins::BaseApplLayer {

public:
    virtual void initialize(int stage);

protected:
    // traci interfaces
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    std::unique_ptr<traci::CommandInterface::Vehicle> plexeTraciVehicle;

    // determines position and role of each vehicle
    BasePositionHelper* positionHelper;

    // controller used by followers
    enum ACTIVE_CONTROLLER controller;

    // list of various controller parameters
    // headway time to be used for the ACC
    double accHeadway;
    // headway time for ACC of leaders
    double leaderHeadway;
    // cacc and engine related parameters
    double caccXi;
    double caccOmegaN;
    double caccC1;
    double caccSpacing;
    double engineTau;
    double uMin, uMax;
    double ploegH;
    double ploegKp;
    double ploegKd;
    double flatbedKa;
    double flatbedKv;
    double flatbedKp;
    double flatbedH;
    double flatbedD;
    bool useControllerAcceleration;
    bool usePrediction;

    // location of the file with vehicle parameters
    std::string vehicleFile;
    // enable/disable realistic engine model
    bool useRealisticEngine;
    // vehicle type for realistic engine model
    std::string vehicleType;

    void initializeControllers();

public:
    BaseScenario()
    {
        mobility = 0;
        traci = 0;
        traciVehicle = 0;
        positionHelper = 0;
        useRealisticEngine = false;
        useControllerAcceleration = true;
        usePrediction = true;
    }

protected:
    virtual void handleSelfMsg(cMessage* msg);
};

} // namespace plexe

#endif
