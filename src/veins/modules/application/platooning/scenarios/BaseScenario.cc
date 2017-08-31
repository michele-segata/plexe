//
// Copyright (c) 2012-2016 Michele Segata <segata@ccs-labs.org>
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

Define_Module(BaseScenario);

void BaseScenario::initialize(int stage) {

	BaseApplLayer::initialize(stage);

	if (stage == 0) {
		accHeadway = par("accHeadway").doubleValue();
		leaderHeadway = par("leaderHeadway").doubleValue();
		caccXi = par("caccXi").doubleValue();
		caccOmegaN = par("caccOmegaN").doubleValue();
		caccC1 = par("caccC1").doubleValue();
		engineTau = par("engineTau").doubleValue();
		ploegH = par("ploegH").doubleValue();
		ploegKp = par("ploegKp").doubleValue();
		ploegKd = par("ploegKd").doubleValue();

		useRealisticEngine = par("useRealisticEngine").boolValue();
		if (useRealisticEngine) {
			vehicleFile = par("vehicleFile").stdstringValue();
			vehicleType = par("vehicleType").stdstringValue();
		}

		const char *strController = par("controller").stringValue();
		//for now we have only two possibilities
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
		else {
			throw cRuntimeError("Invalid controller selected");
		}

	}

	if (stage == 1) {
		mobility = Veins::TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();
		positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());
		initializeControllers();

		//set the active controller
		if (positionHelper->isLeader()) {
			traciVehicle->setActiveController(Plexe::ACC);
			traciVehicle->setACCHeadwayTime(leaderHeadway);
		}
		else {
			traciVehicle->setActiveController(controller);
			traciVehicle->setACCHeadwayTime(accHeadway);
		}
		//set the current lane
		traciVehicle->setFixedLane(positionHelper->getPlatoonLane());

		if (positionHelper->getId() == 0)
			traci->guiView("View #0").trackVehicle(mobility->getExternalId());
	}

}

void BaseScenario::finish() {
	BaseApplLayer::finish();
}

void BaseScenario::handleSelfMsg(cMessage *msg) {
}

void BaseScenario::initializeControllers() {
	//engine lag
	traciVehicle->setGenericInformation(CC_SET_ENGINE_TAU, &engineTau, sizeof(double));
	//PATH's CACC parameters
	traciVehicle->setGenericInformation(CC_SET_CACC_C1, &caccC1, sizeof(double));
	traciVehicle->setGenericInformation(CC_SET_CACC_OMEGA_N, &caccOmegaN, sizeof(double));
	traciVehicle->setGenericInformation(CC_SET_CACC_XI, &caccXi, sizeof(double));
	//Ploeg's parameters
	traciVehicle->setGenericInformation(CC_SET_PLOEG_H, &ploegH, sizeof(double));
	traciVehicle->setGenericInformation(CC_SET_PLOEG_KP, &ploegKp, sizeof(double));
	traciVehicle->setGenericInformation(CC_SET_PLOEG_KD, &ploegKd, sizeof(double));
	//consensus parameters
	int position = positionHelper->getPosition();
	traciVehicle->setGenericInformation(CC_SET_VEHICLE_POSITION, &position, sizeof(int));
	int nCars = positionHelper->getPlatoonSize();
	traciVehicle->setGenericInformation(CC_SET_PLATOON_SIZE, &nCars, sizeof(int));

	Plexe::VEHICLE_DATA vehicleData;
	//initialize own vehicle data
	if (!positionHelper->isLeader()) {
		//my position
		vehicleData.index = positionHelper->getPosition();
		//my length. TODO: take it from SUMO
		vehicleData.length = 4;
		//the rest is all dummy data
		vehicleData.acceleration = 10;
		vehicleData.positionX = 400000;
		vehicleData.positionY = 0;
		vehicleData.speed = 200;
		vehicleData.time = simTime().dbl();
		traciVehicle->setGenericInformation(CC_SET_VEHICLE_DATA, &vehicleData, sizeof(struct Plexe::VEHICLE_DATA));
	}

	if (useRealisticEngine) {
		int engineModel = CC_ENGINE_MODEL_REALISTIC;
		//the order is important
		//1. let sumo instantiate the realistic engine model
		traciVehicle->setGenericInformation(CC_SET_VEHICLE_ENGINE_MODEL, &engineModel, sizeof(engineModel));
		//2. tell the realistic engine model the location of the parameters file
		traciVehicle->setGenericInformation(CC_SET_VEHICLES_FILE, vehicleFile.c_str(), vehicleFile.length() + 1);
		//3. tell the realistic engine model which vehicle (in the specified parameters file) to use
		traciVehicle->setGenericInformation(CC_SET_VEHICLE_MODEL, vehicleType.c_str(), vehicleType.length() + 1);
	}
}
