//
// Copright (c) 2012-2015 Michele Segata <segata@ccs-labs.org>
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

#include "veins/modules/application/platooning/apps/EngineApp.h"

#include "crng.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/messages/MacPkt_m.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"

#include "veins/modules/application/platooning/protocols/BaseProtocol.h"

Define_Module(EngineApp);

void EngineApp::initialize(int stage) {

	BaseApp::initialize(stage);

	if (stage == 1) {

		vehicleType = par("vehicleType").stdstringValue();
		int engineModel = CC_ENGINE_MODEL_REALISTIC;
		traciVehicle->setGenericInformation(CC_SET_VEHICLE_ENGINE_MODEL, &engineModel, sizeof(engineModel));
		traciVehicle->setGenericInformation(CC_SET_VEHICLE_MODEL, vehicleType.c_str(), vehicleType.length() + 1);

		//leader uses the ACC
		traciVehicle->setActiveController(Plexe::ACC);
		//stop the car first
		traciVehicle->setFixedAcceleration(1, -8);
		//every car must run on first lane
		traciVehicle->setFixedLane(0);
		changeSpeed = new cMessage();
		scheduleAt(SimTime(2), changeSpeed);
		brake = new cMessage();
		scheduleAt(SimTime(60), brake);

	}

}

void EngineApp::finish() {
	BaseApp::finish();
	if (changeSpeed) {
		cancelAndDelete(changeSpeed);
		changeSpeed = 0;
	}
	if (brake) {
		cancelAndDelete(brake);
		changeSpeed = 0;
	}
}

void EngineApp::onData(WaveShortMessage *wsm) {
}

void EngineApp::handleSelfMsg(cMessage *msg) {
	//this takes car of feeding data into CACC and reschedule the self message
	BaseApp::handleSelfMsg(msg);

	if (msg == changeSpeed) {
		//request maximum acceleration to see how it really behaves
		traciVehicle->setFixedAcceleration(1, 20);
	}
	if (msg == brake) {
		//slam on the brakes
		traciVehicle->setFixedAcceleration(1, -20);
	}

}

void EngineApp::onBeacon(WaveShortMessage* wsm) {
}
