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

#include "veins/modules/application/platooning/scenarios/BaseScenario.h"

using namespace Veins;

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
            controller = Plexe::ACC;
        }
        else if (strcmp(strController, "CACC") == 0) {
            controller = Plexe::CACC;
        }
        else if (strcmp(strController, "PLOEG") == 0) {
            controller = Plexe::PLOEG;
        }
        else if (strcmp(strController, "CONSENSUS") == 0) {
            controller = Plexe::CONSENSUS;
        }
        else if (strcmp(strController, "FLATBED") == 0) {
            controller = Plexe::FLATBED;
        }
        else {
            throw cRuntimeError("Invalid controller selected");
        }
    }

    if (stage == 1) {
        mobility = Veins::TraCIMobilityAccess().get(getParentModule());
        ASSERT(mobility);
        traci = mobility->getCommandInterface();
        ASSERT(traci);
        traciVehicle = mobility->getVehicleCommandInterface();
        ASSERT(traciVehicle);
        positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());
        initializeControllers();

        // set the active controller
        if (positionHelper->isLeader()) {
            traciVehicle->setActiveController(Plexe::ACC);
            traciVehicle->setACCHeadwayTime(leaderHeadway);
        }
        else {
            traciVehicle->setActiveController(controller);
            traciVehicle->setACCHeadwayTime(accHeadway);
        }
        // set the current lane
        traciVehicle->setFixedLane(positionHelper->getPlatoonLane());
        traciVehicle->setSpeedMode(0);
        traciVehicle->usePrediction(usePrediction);

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
    traciVehicle->setParameter(CC_PAR_CACC_C1, caccC1);
    traciVehicle->setParameter(CC_PAR_CACC_OMEGA_N, caccOmegaN);
    traciVehicle->setParameter(CC_PAR_CACC_XI, caccXi);
    // Ploeg's parameters
    traciVehicle->setParameter(CC_PAR_PLOEG_H, ploegH);
    traciVehicle->setParameter(CC_PAR_PLOEG_KP, ploegKp);
    traciVehicle->setParameter(CC_PAR_PLOEG_KD, ploegKd);
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
    traciVehicle->useControllerAcceleration(useControllerAcceleration);

    Plexe::VEHICLE_DATA vehicleData;
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
        traciVehicle->setVehicleData(&vehicleData);
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
