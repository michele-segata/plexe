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

#include "plexe/scenarios/BaseScenario.h"

#include "veins/base/utils/FindModule.h"

#include "plexe/PlexeManager.h"

using namespace veins;

namespace plexe {

Define_Module(BaseScenario);

void BaseScenario::initialize(int stage)
{

    BaseApplLayer::initialize(stage);

    if (stage == 0) {
        accHeadway = par("accHeadway").doubleValue();
        leaderHeadway = par("leaderHeadway").doubleValue();
        caccXi = par("caccXi").doubleValue();
        caccOmegaN = par("caccOmegaN").doubleValue();
        caccC1 = par("caccC1").doubleValue();
        caccSpacing = par("caccSpacing").doubleValueInUnit("m");
        engineTau = par("engineTau").doubleValue();
        uMin = par("uMin").doubleValue();
        uMax = par("uMax").doubleValue();
        ploegH = par("ploegH").doubleValue();
        ploegKp = par("ploegKp").doubleValue();
        ploegKd = par("ploegKd").doubleValue();
        flatbedKa = par("flatbedKa").doubleValue();
        flatbedKv = par("flatbedKv").doubleValue();
        flatbedKp = par("flatbedKp").doubleValue();
        flatbedH = par("flatbedH").doubleValue();
        flatbedD = par("flatbedD").doubleValue();
        useControllerAcceleration = par("useControllerAcceleration").boolValue();
        usePrediction = par("usePrediction").boolValue();

        useRealisticEngine = par("useRealisticEngine").boolValue();
        if (useRealisticEngine) {
            vehicleFile = par("vehicleFile").stdstringValue();
            vehicleType = par("vehicleType").stdstringValue();
        }

        const char* strController = par("controller").stringValue();
        // for now we have only two possibilities
        if (strcmp(strController, "ACC") == 0) {
            controller = ACC;
        }
        else if (strcmp(strController, "CACC") == 0) {
            controller = CACC;
        }
        else if (strcmp(strController, "PLOEG") == 0) {
            controller = PLOEG;
        }
        else if (strcmp(strController, "CONSENSUS") == 0) {
            controller = CONSENSUS;
        }
        else if (strcmp(strController, "FLATBED") == 0) {
            controller = FLATBED;
        }
        else {
            throw cRuntimeError("Invalid controller selected");
        }
    }

    if (stage == 1) {
        mobility = veins::TraCIMobilityAccess().get(getParentModule());
        ASSERT(mobility);
        traci = mobility->getCommandInterface();
        ASSERT(traci);
        traciVehicle = mobility->getVehicleCommandInterface();
        ASSERT(traciVehicle);
        auto plexe = FindModule<PlexeManager*>::findGlobalModule();
        ASSERT(plexe);
        plexeTraci = plexe->getCommandInterface();
        plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, mobility->getExternalId()));
        positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());
        initializeControllers();

        // set the active controller
        if (positionHelper->isLeader()) {
            plexeTraciVehicle->setActiveController(ACC);
            plexeTraciVehicle->setACCHeadwayTime(leaderHeadway);
        }
        else {
            plexeTraciVehicle->setActiveController(controller);
            plexeTraciVehicle->setACCHeadwayTime(accHeadway);
        }
        // set the current lane
        plexeTraciVehicle->setFixedLane(positionHelper->getPlatoonLane());
        traciVehicle->setSpeedMode(0);
        plexeTraciVehicle->usePrediction(usePrediction);

        if (positionHelper->getId() == 0) traci->guiView("View #0").trackVehicle(mobility->getExternalId());
    }
}

void BaseScenario::handleSelfMsg(cMessage* msg)
{
}

void BaseScenario::initializeControllers()
{
    // engine lag
    traciVehicle->setParameter(CC_PAR_ENGINE_TAU, engineTau);
    traciVehicle->setParameter(CC_PAR_UMIN, uMin);
    traciVehicle->setParameter(CC_PAR_UMAX, uMax);
    // PATH's CACC parameters
    plexeTraciVehicle->setPathCACCParameters(caccOmegaN, caccXi, caccC1, caccSpacing);
    // Ploeg's parameters
    plexeTraciVehicle->setPloegCACCParameters(ploegKp, ploegKd, ploegH);
    // flatbed's parameters
    traciVehicle->setParameter(CC_PAR_FLATBED_KA, flatbedKa);
    traciVehicle->setParameter(CC_PAR_FLATBED_KV, flatbedKv);
    traciVehicle->setParameter(CC_PAR_FLATBED_KP, flatbedKp);
    traciVehicle->setParameter(CC_PAR_FLATBED_H, flatbedH);
    traciVehicle->setParameter(CC_PAR_FLATBED_D, flatbedD);
    // consensus parameters
    traciVehicle->setParameter(CC_PAR_VEHICLE_POSITION, positionHelper->getPosition());
    traciVehicle->setParameter(CC_PAR_PLATOON_SIZE, positionHelper->getPlatoonSize());
    // use of controller acceleration
    plexeTraciVehicle->useControllerAcceleration(useControllerAcceleration);

    VEHICLE_DATA vehicleData;
    // initialize own vehicle data
    if (!positionHelper->isLeader()) {
        // my position
        vehicleData.index = positionHelper->getPosition();
        // my length
        vehicleData.length = traciVehicle->getLength();
        // the rest is all dummy data
        vehicleData.acceleration = 10;
        vehicleData.positionX = 400000;
        vehicleData.positionY = 0;
        vehicleData.speed = 200;
        vehicleData.time = simTime().dbl();
        vehicleData.u = 0;
        plexeTraciVehicle->setVehicleData(&vehicleData);
    }

    if (useRealisticEngine) {
        int engineModel = CC_ENGINE_MODEL_REALISTIC;
        // the order is important
        // 1. let sumo instantiate the realistic engine model
        traciVehicle->setParameter(CC_PAR_VEHICLE_ENGINE_MODEL, engineModel);
        // 2. tell the realistic engine model the location of the parameters file
        traciVehicle->setParameter(CC_PAR_VEHICLES_FILE, vehicleFile);
        // 3. tell the realistic engine model which vehicle (in the specified parameters file) to use
        traciVehicle->setParameter(CC_PAR_VEHICLE_MODEL, vehicleType);
    }
}

} // namespace plexe
