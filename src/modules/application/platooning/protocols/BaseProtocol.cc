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

#include "BaseProtocol.h"

#include <Mac1609_4.h>

Define_Module(BaseProtocol)

void BaseProtocol::initialize(int stage) {

	BaseApplLayer::initialize(stage);

	if (stage == 0) {

		//init class variables
		sendBeacon = 0;
		seq_n = 0;
		dataPolling = 0;

		//get gates
		upperLayerIn = findGate("upperLayerIn");
		upperLayerOut = findGate("upperLayerOut");
		lowerControlIn = findGate("lowerControlIn");
		lowerControlOut = findGate("lowerControlOut");
		lowerLayerIn = findGate("lowerLayerIn");
		lowerLayerOut = findGate("lowerLayerOut");

		//this is the id of the vehicle. used also as network address
		myId = getParentModule()->getIndex();

		//tell the unicast protocol below which mac address to use via control message
		UnicastProtocolControlMessage *setMacAddress = new UnicastProtocolControlMessage("");
		setMacAddress->setControlCommand(SET_MAC_ADDRESS);
		setMacAddress->setCommandValue(myId);
		send(setMacAddress, lowerControlOut);

		//beaconing interval in seconds
		beaconingInterval = SimTime(par("beaconingInterval").doubleValue());
		//platooning message packet size
		packetSize = par("packetSize").longValue();
		//priority of platooning message
		priority = par("priority").longValue();
		ASSERT2(priority >= 0 && priority <= 3, "priority value must be between 0 and 3");

		//when to stop simulation (after communications started)
		communicationDuration = SimTime(par("communicationDuration").longValue());
		//use controller or real acceleration?
		useControllerAcceleration = par("useControllerAcceleration").boolValue();

		//init messages for scheduleAt
		sendBeacon = new cMessage("sendBeacon");
		dataPolling = new cMessage("dataPolling");

		//get traci interface
		traci = TraCIMobilityAccess().get(getParentModule());

		//set names for output vectors
		//distance from front vehicle
		distanceOut.setName("distance");
		//relative speed w.r.t. front vehicle
		relSpeedOut.setName("relativeSpeed");
		//vehicle id
		nodeIdOut.setName("nodeId");
		//current speed
		speedOut.setName("speed");
		//vehicle position
		posxOut.setName("posx");
		posyOut.setName("posy");
		//vehicle acceleration
		accelerationOut.setName("acceleration");

		//init data polling. do it at each tenth of a second
		scheduleAt(SimTime(((int)((simTime().dbl() + .1) * 10)) / 10.0), dataPolling);

	}

}

void BaseProtocol::finish() {
	if (sendBeacon) {
		if (sendBeacon->isScheduled()) {
			cancelEvent(sendBeacon);
		}
		delete sendBeacon;
	}
	if (dataPolling) {
		if (dataPolling->isScheduled()) {
			cancelEvent(dataPolling);
		}
		delete dataPolling;
	}
}

void BaseProtocol::handleSelfMsg(cMessage *msg) {

	if (msg == dataPolling) {

		//check for simulation end. let the first vehicle check
		if (myId == 0 && simTime() > communicationDuration) {
			endSimulation();
		}

		//get distance and relative speed w.r.t. front vehicle
		double distance, relSpeed, acceleration, speed, controllerAcceleration, posX, posY, time;
		traci->commandGetRadarMeasurements(traci->getExternalId(), distance, relSpeed);
		traci->commandGetVehicleData(traci->getExternalId(), speed, acceleration, controllerAcceleration, posX, posY, time);

		//write data to output files
		distanceOut.record(distance);
		relSpeedOut.record(relSpeed);
		nodeIdOut.record(myId);
		accelerationOut.record(acceleration);
		speedOut.record(traci->getCurrentSpeed().x);
		Coord pos = traci->getPositionAt(simTime());
		posxOut.record(pos.x);
		posyOut.record(pos.y);

		scheduleAt(simTime() + SimTime(0.1), dataPolling);

	}

}

void BaseProtocol::sendPlatooningMessage(int destinationAddress) {

	//vehicle's data to be included in the message
	double speed, acceleration, controllerAcceleration, sumoPosX, sumoPosY, sumoTime;

	//actual packets
	UnicastMessage *unicast;
	PlatooningBeacon *pkt;
	//get information about the vehicle via traci
	this->traci->commandGetVehicleData(this->traci->getExternalId(), speed, acceleration, controllerAcceleration, sumoPosX, sumoPosY, sumoTime);
	//get current vehicle position
	Coord veinsPosition = this->traci->getPositionAt(simTime());
	double veinsTime = simTime().dbl();

	//TODO: use veins or sumo data?
	Coord position(veinsPosition.x, veinsPosition.y, veinsPosition.z);
	double time = veinsTime;

	//create and send beacon
	unicast = new UnicastMessage("", BEACON_TYPE);
	unicast->setDestination(-1);
	unicast->setPriority(priority);

	//create platooning beacon with data about the car
	pkt = new PlatooningBeacon();
	if (useControllerAcceleration) {
		pkt->setAcceleration(controllerAcceleration);
	}
	else {
		pkt->setAcceleration(acceleration);
	}
	pkt->setSpeed(speed);
	pkt->setVehicleId(myId);
	pkt->setPositionX(position.x);
	pkt->setPositionY(position.y);
	//set the time to now
	pkt->setTime(time);
	//i generated the message, i send it
	pkt->setRelayerId(myId);
	pkt->setKind(BEACON_TYPE);
	pkt->setByteLength(packetSize);
	pkt->setSequenceNumber(seq_n++);

	//put platooning beacon into the message for the UnicastProtocol
	unicast->encapsulate(pkt);
	sendDown(unicast);

}

void BaseProtocol::handleUnicastMsg(UnicastMessage *unicast) {

	PlatooningBeacon *epkt;

	ASSERT2(unicast, "received a frame not of type UnicastMessage");

	cPacket *enc = unicast->decapsulate();
	ASSERT2(enc, "received a UnicastMessage with nothing inside");

	epkt = dynamic_cast<PlatooningBeacon *>(enc);

	if (epkt) {

		//invoke messageReceived() method of subclass
		messageReceived(epkt, unicast);

	}

	//send the message to the platooning application
	UnicastMessage *duplicate = unicast->dup();
	duplicate->encapsulate(epkt->dup());
	send(duplicate, upperLayerOut);

	delete enc;

}

void BaseProtocol::handleLowerMsg(cMessage *msg) {

	UnicastMessage *unicast;

	unicast = dynamic_cast<UnicastMessage *>(msg);
	handleUnicastMsg(unicast);
	delete unicast;

}

void BaseProtocol::handleUpperMsg(cMessage *msg) {
	UnicastMessage *unicast;
	unicast = dynamic_cast<UnicastMessage *>(msg);
	assert(unicast);
	sendDown(msg);
}

void BaseProtocol::handleUpperControl(cMessage *msg) {
	UnicastProtocolControlMessage *ctrl = dynamic_cast<UnicastProtocolControlMessage *>(msg);
	if (ctrl) {
		if (ctrl->getControlCommand() == SET_MAC_ADDRESS) {
			//set id to be the address we want to set to the NIC card
			myId = ctrl->getCommandValue();
		}
		sendControlDown(ctrl);
	}
}

void BaseProtocol::handleLowerControl(cMessage *msg) {
	UnicastProtocolControlMessage *ctrl = dynamic_cast<UnicastProtocolControlMessage *>(msg);
	if (ctrl) {
		sendControlUp(ctrl);
	}
}

void BaseProtocol::messageReceived(PlatooningBeacon *pkt, UnicastMessage *unicast) {
	ASSERT2(false, "BaseProtocol::messageReceived() not overridden by subclass");
}

BaseProtocol::~BaseProtocol() {
}

