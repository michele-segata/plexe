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

#ifndef BASESCENARIO_H_
#define BASESCENARIO_H_

#include "veins/base/modules/BaseApplLayer.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/modules/application/platooning/CC_Const.h"

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

class BaseScenario : public Veins::BaseApplLayer {

public:
    virtual void initialize(int stage);

protected:
    // traci interfaces
    Veins::TraCIMobility* mobility;
    Veins::TraCICommandInterface* traci;
    Veins::TraCICommandInterface::Vehicle* traciVehicle;

    // determines position and role of each vehicle
    BasePositionHelper* positionHelper;

    // controller used by followers
    enum Plexe::ACTIVE_CONTROLLER controller;

    // list of various controller parameters
    // headway time to be used for the ACC
    double accHeadway;
    // headway time for ACC of leaders
    double leaderHeadway;
    // cacc and engine related parameters
    double caccXi;
    double caccOmegaN;
    double caccC1;
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

#endif
