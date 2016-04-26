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

#include "veins/modules/application/platooning/protocols/BaseProtocol.h"

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"

Define_Module(BaseProtocol)

//set signals for channel busy and collisions
const simsignalwrap_t BaseProtocol::sigChannelBusy = simsignalwrap_t("sigChannelBusy");
const simsignalwrap_t BaseProtocol::sigCollision = simsignalwrap_t("sigCollision");

void BaseProtocol::initialize(int stage) {

	BaseApplLayer::initialize(stage);

	if (stage == 0) {

		//init class variables
		sendBeacon = 0;
		channelBusy = false;
		nCollisions = 0;
		busyTime = SimTime(0);
		seq_n = 0;
		recordData = 0;

		//get gates
		lowerControlIn = findGate("lowerControlIn");
		lowerControlOut = findGate("lowerControlOut");
		lowerLayerIn = findGate("lowerLayerIn");
		lowerLayerOut = findGate("lowerLayerOut");
		minUpperId = gate("upperLayerIn", 0)->getId();
		maxUpperId = gate("upperLayerIn", MAX_APPLICATIONS_COUNT - 1)->getId();

		//get traci interface
		mobility = Veins::TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();
		positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());

		//this is the id of the vehicle. used also as network address
		myId = positionHelper->getId();

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

		//use controller or real acceleration?
		useControllerAcceleration = par("useControllerAcceleration").boolValue();

		//init messages for scheduleAt
		sendBeacon = new cMessage("sendBeacon");
		recordData = new cMessage("recordData");

		//set names for output vectors
		//own id
		nodeIdOut.setName("nodeId");
		//channel busy time
		busyTimeOut.setName("busyTime");
		//mac layer collisions
		collisionsOut.setName("collisions");
		//delay metrics
		lastLeaderMsgTime = SimTime(-1);
		lastFrontMsgTime = SimTime(-1);
		leaderDelayIdOut.setName("leaderDelayId");
		frontDelayIdOut.setName("frontDelayId");
		leaderDelayOut.setName("leaderDelay");
		frontDelayOut.setName("frontDelay");

		//subscribe to signals for channel busy state and collisions
		findHost()->subscribe(sigChannelBusy, this);
		findHost()->subscribe(sigCollision, this);

		//init statistics collection. round to second
		SimTime rounded = SimTime(floor(simTime().dbl() + 1), SIMTIME_S);
		scheduleAt(rounded, recordData);

	}

}

void BaseProtocol::finish() {
	if (sendBeacon) {
		if (sendBeacon->isScheduled()) {
			cancelEvent(sendBeacon);
		}
		delete sendBeacon;
		sendBeacon = 0;
	}
	if (recordData) {
		if (recordData->isScheduled()) {
			cancelEvent(recordData);
		}
		delete recordData;
		recordData = 0;
	}
	BaseApplLayer::finish();
}

void BaseProtocol::handleSelfMsg(cMessage *msg) {

	if (msg == recordData) {

		//if channel is currently busy, we have to split the amount of time between
		//this period and the successive. so we just compute the channel busy time
		//up to now, and then reset the "startBusy" timer to now
		if (channelBusy) {
			busyTime += simTime() - startBusy;
			startBusy = simTime();
		}

		//time for writing statistics
		//node id
		nodeIdOut.record(myId);
		//record busy time for this period
		busyTimeOut.record(busyTime);
		//record collisions for this period
		collisionsOut.record(nCollisions);

		//and reset counter
		busyTime = SimTime(0);
		nCollisions = 0;

		scheduleAt(simTime() + SimTime(1, SIMTIME_S), recordData);

	}

}

void BaseProtocol::sendPlatooningMessage(int destinationAddress) {

	//vehicle's data to be included in the message
	double speed, acceleration, controllerAcceleration, sumoPosX, sumoPosY, sumoTime;

	//actual packets
	UnicastMessage *unicast;
	PlatooningBeacon *pkt;
	//get information about the vehicle via traci
	traciVehicle->getVehicleData(speed, acceleration, controllerAcceleration, sumoPosX, sumoPosY, sumoTime);
	//get current vehicle position
	Coord veinsPosition = mobility->getPositionAt(simTime());
	//transform veins position into sumo position
	Veins::TraCICoord coords = mobility->getManager()->omnet2traci(veinsPosition);
	double veinsTime = simTime().dbl();

	Coord position(coords.x, coords.y, 0);
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

		if (positionHelper->getLeaderId() == epkt->getVehicleId()) {
			//check if this is at least the second message we have received
			if (lastLeaderMsgTime.dbl() > 0) {
				leaderDelayOut.record(simTime() - lastLeaderMsgTime);
				leaderDelayIdOut.record(myId);
			}
			lastLeaderMsgTime = simTime();

		}
		if (positionHelper->getFrontId() == epkt->getVehicleId()) {
			//check if this is at least the second message we have received
			if (lastFrontMsgTime.dbl() > 0) {
				frontDelayOut.record(simTime() - lastFrontMsgTime);
				frontDelayIdOut.record(myId);
			}
			lastFrontMsgTime = simTime();
		}

	}

	//find the application responsible for this beacon
	ApplicationMap::iterator app = apps.find(unicast->getKind());
	if (app != apps.end()) {
		//send the message to the application responsible for it
		UnicastMessage *duplicate = unicast->dup();
		duplicate->encapsulate(enc->dup());
		send(duplicate, app->second.second);
	}

	delete enc;

}

void BaseProtocol::receiveSignal(cComponent *source, simsignal_t signalID, bool v, cObject *details) {

	Enter_Method_Silent();
	if (signalID == sigChannelBusy) {
		if (v && !channelBusy) {
			//channel turned busy, was idle before
			startBusy = simTime();
			channelBusy = true;
			channelBusyStart();
			return;
		}
		if (!v && channelBusy) {
			//channel turned idle, was busy before
			busyTime += simTime() - startBusy;
			channelBusy = false;
			channelIdleStart();
			return;
		}
	}
	if (signalID == sigCollision) {
		collision();
		nCollisions++;
	}

}

void BaseProtocol::handleMessage(cMessage *msg) {
	if (msg->getArrivalGateId() >= minUpperId && msg->getArrivalGateId() <= maxUpperId)
		handleUpperMsg(msg);
	else
		BaseApplLayer::handleMessage(msg);
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

void BaseProtocol::registerApplication(int applicationId, cGate* appInputGate, cGate* appOutputGate) {
	//check if an already registered id exists
	ApplicationMap::iterator app = apps.find(applicationId);
	if (app != apps.end())
		throw cRuntimeError("BaseProtocol: application with id=%d already registered. Are you double-registering or do you have duplicated application IDs?", applicationId);
	int nApps = apps.size();
	if (nApps == MAX_APPLICATIONS_COUNT)
		throw cRuntimeError("BaseProtocol: application with id=%d tried to register, but no space left", applicationId);
	//connect gates
	cGate *upperIn, *upperOut;
	upperOut = gate("upperLayerOut", nApps);
	upperOut->connectTo(appInputGate);
	upperIn = gate("upperLayerIn", nApps);
	appOutputGate->connectTo(upperIn);
	//save the mapping in the connection
	apps[applicationId] = AppInOut(upperIn, upperOut);
}

BaseProtocol::~BaseProtocol() {
}

