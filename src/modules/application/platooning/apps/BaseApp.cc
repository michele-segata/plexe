//
// Copright (c) 2012-2014 Michele Segata <segata@ccs-labs.org>
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

#include "BaseApp.h"

#include "crng.h"
#include "WaveShortMessage_m.h"
#include "MacPkt_m.h"
#include "Mac1609_4.h"

#include <BaseProtocol.h>

Define_Module(BaseApp);

void BaseApp::initialize(int stage) {

	BaseApplLayer::initialize(stage);

	if (stage == 0) {

		//init class variables
		useControllerAcceleration = par("useControllerAcceleration").boolValue();
		caccC1 = par("caccC1").doubleValue();
		caccXi = par("caccXi").doubleValue();
		caccOmegaN = par("caccOmegaN").doubleValue();
		engineTau = par("engineTau").doubleValue();
		myccKd = par("myccKd").doubleValue();
		myccKs = par("myccKs").doubleValue();

		myId = getParentModule()->getIndex();
	}

	if (stage == 1) {

		traci = TraCIMobilityAccess().get(getParentModule());
		traci->commandSetGenericInformation(traci->getExternalId(), CC_SET_CACC_C1, &caccC1, sizeof(double));
		traci->commandSetGenericInformation(traci->getExternalId(), CC_SET_CACC_OMEGA_N, &caccOmegaN, sizeof(double));
		traci->commandSetGenericInformation(traci->getExternalId(), CC_SET_CACC_XI, &caccXi, sizeof(double));
		traci->commandSetGenericInformation(traci->getExternalId(), CC_SET_ENGINE_TAU, &engineTau, sizeof(double));
		traci->commandSetGenericInformation(traci->getExternalId(), CC_SET_MYCC_KD, &myccKd, sizeof(double));
		traci->commandSetGenericInformation(traci->getExternalId(), CC_SET_MYCC_KS, &myccKs, sizeof(double));

	}

}

void BaseApp::finish() {
	BaseApplLayer::finish();
}

void BaseApp::handleLowerMsg(cMessage *msg) {

	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	ASSERT2(unicast, "received a frame not of type UnicastMessage");

	cPacket *enc = unicast->decapsulate();
	ASSERT2(enc, "received a UnicastMessage with nothing inside");

	if (enc->getKind() == BaseProtocol::BEACON_TYPE) {

		PlatooningBeacon *epkt = dynamic_cast<PlatooningBeacon *>(enc);
		ASSERT2(epkt, "received UnicastMessage does not contain a PlatooningBeacon");

		//if the message comes from the leader
		if (epkt->getVehicleId() == 0) {
			traci->commandSetPlatoonLeaderData(traci->getExternalId(), epkt->getSpeed(), epkt->getAcceleration(), epkt->getPositionX(), epkt->getPositionY(), epkt->getTime());
		}
		//if the message comes from the vehicle in front
		if (epkt->getVehicleId() == myId - 1) {
			traci->commandSetPrecedingVehicleData(traci->getExternalId(), epkt->getSpeed(), epkt->getAcceleration(), epkt->getPositionX(), epkt->getPositionY(), epkt->getTime());
		}

	}

	delete enc;
	delete unicast;
}

void BaseApp::handleLowerControl(cMessage *msg) {
	delete msg;
}

void BaseApp::onData(WaveShortMessage *wsm) {
}

void BaseApp::sendUnicast(cPacket *msg, int destination) {
	UnicastMessage *unicast = new UnicastMessage();
	unicast->setDestination(destination);
	unicast->encapsulate(msg);
	sendDown(unicast);
}

void BaseApp::handleSelfMsg(cMessage *msg) {
}

void BaseApp::onBeacon(WaveShortMessage* wsm) {
}

std::string BaseApp::getExternalId() {
	return traci->getExternalId();
}
