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

#include "veins/modules/application/platooning/maneuver/JoinManeuverScenario.h"

Define_Module(JoinManeuverScenario);

void JoinManeuverScenario::initialize(int stage) {

	BaseScenario::initialize(stage);

	if (stage == 0) {
		leaderFsm = LS_LEADING;
		joinerFsm = JS_IDLE;
		followerFsm = FS_FOLLOW;
	}

	if (stage == 1) {

		//lane where the platoon is driving
		int platoonLane = 0;

		prepareManeuverCars(platoonLane);

		protocol = FindModule<BaseProtocol*>::findSubModule(getParentModule());

		//connect maneuver application to protocol
		protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
		//we are also interested in receiving beacons: the joiner must compute
		//its distance to the front vehicle while approaching it
		protocol->registerApplication(BaseProtocol::BEACON_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
	}

}

void JoinManeuverScenario::prepareManeuverCars(int platoonLane) {

	switch (positionHelper->getId()) {

		case 0: {
			//this is the leader
			traciVehicle->setCruiseControlDesiredSpeed(100.0 / 3.6);
			traciVehicle->setActiveController(Plexe::ACC);
			traciVehicle->setFixedLane(platoonLane);
			role = LEADER;

			positionHelper->setLeaderId(positionHelper->getId());
			positionHelper->setIsLeader(true);
			positionHelper->setPlatoonLane(platoonLane);
			positionHelper->setPlatoonId(positionHelper->getId());

			vehicleData.joinerId = -1;
			vehicleData.speed = 100/3.6;
			vehicleData.formation.push_back(0);
			vehicleData.formation.push_back(1);
			vehicleData.formation.push_back(2);
			vehicleData.formation.push_back(3);

			break;
		}

		case 1:
		case 2:
		case 3: {
			//these are the followers which are already in the platoon
			traciVehicle->setCruiseControlDesiredSpeed(130.0 / 3.6);
			traciVehicle->setActiveController(Plexe::CACC);
			traciVehicle->setFixedLane(platoonLane);
			role = FOLLOWER;

			positionHelper->setLeaderId(0);
			positionHelper->setFrontId(positionHelper->getId() - 1);
			positionHelper->setIsLeader(false);
			positionHelper->setPlatoonLane(platoonLane);
			positionHelper->setPlatoonId(positionHelper->getLeaderId());

			vehicleData.speed = 100/3.6;
			vehicleData.formation.push_back(0);
			vehicleData.formation.push_back(1);
			vehicleData.formation.push_back(2);
			vehicleData.formation.push_back(3);


			break;
		}

		case 4: {
			//this is the car which will join
			traciVehicle->setCruiseControlDesiredSpeed(100/3.6);
			traciVehicle->setFixedLane(2);
			traciVehicle->setActiveController(Plexe::ACC);
			role = JOINER;

			//we assume leader and platoon id to be known, as it is the one we want to join
			positionHelper->setLeaderId(0);
			positionHelper->setPlatoonId(positionHelper->getLeaderId());
			positionHelper->setFrontId(-1);
			positionHelper->setIsLeader(false);
			positionHelper->setPlatoonLane(-1);

			vehicleData.speed = 100/3.6;

			//after 30 seconds of simulation, start the maneuver
			startManeuver = new cMessage();
			scheduleAt(simTime() + SimTime(10), startManeuver);
			break;
		}
	}

}

void JoinManeuverScenario::finish() {

	if (startManeuver) {
		cancelAndDelete(startManeuver);
		startManeuver = 0;
	}

	BaseScenario::finish();

}

ManeuverMessage *JoinManeuverScenario::generateMessage() {
	ManeuverMessage *msg = new ManeuverMessage();
	msg->setVehicleId(positionHelper->getId());
	msg->setPlatoonId(positionHelper->getPlatoonId());
	msg->setPlatoonLane(positionHelper->getPlatoonLane());
	msg->setPlatoonSpeed(vehicleData.speed);
	return msg;
}

void JoinManeuverScenario::handleSelfMsg(cMessage *msg) {

	//this takes car of feeding data into CACC and reschedule the self message
	BaseScenario::handleSelfMsg(msg);

	if (msg == startManeuver)
		handleJoinerMsg(msg);

}

void JoinManeuverScenario::handleLowerMsg(cMessage *msg) {
	switch (role) {
		case LEADER:
			handleLeaderMsg(msg);
			break;
		case FOLLOWER:
			handleFollowerMsg(msg);
			break;
		case JOINER:
			handleJoinerMsg(msg);
			break;
		default:
			ASSERT(false);
			break;
	};

}

void JoinManeuverScenario::sendUnicast(cPacket *msg, int destination) {
	UnicastMessage *unicast = new UnicastMessage("", MANEUVER_TYPE);
	unicast->setDestination(destination);
	unicast->setChannel(Channels::CCH);
	unicast->encapsulate(msg);
	sendDown(unicast);
}

void JoinManeuverScenario::handleLeaderMsg(cMessage *msg) {

	//this message can be a self message, or a unicast message
	//with an encapsulated beacon or maneuver message
	ManeuverMessage *maneuver = 0;
	cPacket *encapsulated = 0;
	//maneuver message to be sent, if needed
	ManeuverMessage *toSend;

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
	}

	//check current leader status
	switch (leaderFsm) {
		case LS_LEADING: {
			//when getting a message, and being in the LEADING state, we need
			//to check if this is a join request. if not just ignore it
			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				if (maneuver->getMessageType() == JM_REQUEST_JOIN) {

					toSend = generateMessage();
					toSend->setMessageType(LM_MOVE_IN_POSITION);
					//this will be the front vehicle for the car which will join
					toSend->setFrontVehicleId(3);
					//save some data. who is joining?
					vehicleData.joinerId = maneuver->getVehicleId();
					//send a positive ack to the joiner
					sendUnicast(toSend, vehicleData.joinerId);

					leaderFsm = LS_WAIT_JOINER_IN_POSITION;
				}
			}
			break;
		}
		case LS_WAIT_JOINER_IN_POSITION: {

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				//the joiner is now in position and is ready to join
				if (maneuver->getMessageType() == JM_IN_POSITION) {

					//tell him to join the platoon
					toSend = generateMessage();
					toSend->setMessageType(LM_JOIN_PLATOON);
					sendUnicast(toSend, vehicleData.joinerId);

					leaderFsm = LS_WAIT_JOINER_TO_JOIN;

				}
			}

			break;
		}
		case LS_WAIT_JOINER_TO_JOIN: {

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				//the joiner has joined the platoon
				if (maneuver->getMessageType() == JM_IN_PLATOON) {
					//add the joiner to the list of vehicles in the platoon
					vehicleData.formation.push_back(vehicleData.joinerId);
					toSend = generateMessage();
					toSend->setMessageType(LM_UPDATE_FORMATION);
					toSend->setPlatoonFormationArraySize(vehicleData.formation.size());
					for (unsigned int i = 0; i < vehicleData.formation.size(); i++) {
						toSend->setPlatoonFormation(i, vehicleData.formation[i]);
					}
					//send to all vehicles
					sendUnicast(toSend, -1);

					leaderFsm = LS_LEADING;

				}

			}

			break;
		}

	}

	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	}

}

void JoinManeuverScenario::handleJoinerMsg(cMessage *msg) {

	//this message can be a self message, or a unicast message
	//with an encapsulated beacon or maneuver message
	ManeuverMessage *maneuver = 0;
	PlatooningBeacon *beacon = 0;
	cPacket *encapsulated = 0;
	//maneuver message to be sent, if needed
	ManeuverMessage *toSend;

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
		beacon = dynamic_cast<PlatooningBeacon *>(encapsulated);
	}

	//check current joiner status
	switch(joinerFsm) {

		case JS_IDLE: {
			//if this is a self message triggering the beginning of procedure, then ask for joining
			if (msg == startManeuver) {
				toSend = generateMessage();
				toSend->setMessageType(JM_REQUEST_JOIN);
				sendUnicast(toSend, positionHelper->getLeaderId());
				joinerFsm = JS_WAIT_REPLY;
			}
			break;
		}

		case JS_WAIT_REPLY: {

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {

				//if the leader told us to move in position, we can start approaching the platoon
				if (maneuver->getMessageType() == LM_MOVE_IN_POSITION) {
					//save some data about the platoon
					positionHelper->setFrontId(maneuver->getFrontVehicleId());
					vehicleData.joinLane = maneuver->getPlatoonLane();

					//check for correct lane. if not in correct lane, change it
					int currentLane = traciVehicle->getLaneIndex();
					if (currentLane != vehicleData.joinLane) {
						traciVehicle->setFixedLane(vehicleData.joinLane);
					}

					//activate faked CACC. this way we can approach the front car using data obtained through GPS
					traciVehicle->setCACCConstantSpacing(15);
					//we have no data so far, so for the moment just initialize with some fake data
					traciVehicle->setLeaderVehicleFakeData(0, 0, vehicleData.speed);
					traciVehicle->setFrontVehicleFakeData(0, 0, vehicleData.speed, 15);
					//set a CC speed higher than the platoon speed to approach it
					traciVehicle->setCruiseControlDesiredSpeed(vehicleData.speed + 30/3.6);
					traciVehicle->setActiveController(Plexe::FAKED_CACC);
					joinerFsm = JS_MOVE_IN_POSITION;
				}

			}
			break;
		}

		case JS_MOVE_IN_POSITION: {

			//if we get data, just feed the fake CACC
			if (beacon && beacon->getVehicleId() == positionHelper->getFrontId()) {
				//get front vehicle position
				Coord frontPosition(beacon->getPositionX(), beacon->getPositionY(), 0);
				//get my position
				Veins::TraCICoord traciPosition = mobility->getManager()->omnet2traci(mobility->getCurrentPosition());
				Coord position(traciPosition.x, traciPosition.y);
				//compute distance (-4 because of vehicle length)
				double distance = position.distance(frontPosition) - 4;
				//if we are in position, tell the leader about that
				if (distance < 16) {
					toSend = generateMessage();
					toSend->setMessageType(JM_IN_POSITION);
					sendUnicast(toSend, positionHelper->getLeaderId());
					joinerFsm = JS_WAIT_JOIN;
				}
			}
			break;
		}

		case JS_WAIT_JOIN: {

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {

				//if we get confirmation from the leader, switch from faked CACC to real CACC
				if (maneuver->getMessageType() == LM_JOIN_PLATOON) {
					traciVehicle->setActiveController(Plexe::CACC);
					//set spacing to 5 meters to get close to the platoon
					traciVehicle->setCACCConstantSpacing(5);
				}
				//tell the leader that we're now in the platoon
				toSend = generateMessage();
				toSend->setMessageType(JM_IN_PLATOON);
				sendUnicast(toSend, positionHelper->getLeaderId());
				joinerFsm = JS_FOLLOW;

			}

			break;

		}

		case JS_FOLLOW: {

			//we're now following. if we get an update of the formation, change it accordingly
			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				if (maneuver->getMessageType() == LM_UPDATE_FORMATION) {
					vehicleData.formation.clear();
					for (unsigned int i = 0; i < maneuver->getPlatoonFormationArraySize(); i++) {
						vehicleData.formation.push_back(maneuver->getPlatoonFormation(i));
					}
				}
			}

			break;
		}

	}

	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	}

}

void JoinManeuverScenario::handleFollowerMsg(cMessage *msg) {

	//this message can be a self message, or a unicast message
	//with an encapsulated beacon or maneuver message
	ManeuverMessage *maneuver = 0;
	cPacket *encapsulated = 0;

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
	}

	//check current follower status
	switch(followerFsm) {

		case FS_FOLLOW: {

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				if (maneuver->getMessageType() == LM_UPDATE_FORMATION) {
					vehicleData.formation.clear();
					for (unsigned int i = 0; i < maneuver->getPlatoonFormationArraySize(); i++) {
						vehicleData.formation.push_back(maneuver->getPlatoonFormation(i));
					}
				}
			}

			break;
		}

	}

	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	}

}

void JoinManeuverScenario::handleLowerControl(cMessage *msg) {
	//lower control message
	UnicastProtocolControlMessage *ctrl = 0;

	ctrl = dynamic_cast<UnicastProtocolControlMessage *>(msg);
	//TODO: check for double free corruption
	if (ctrl) {
		delete ctrl;
	}
	else {
		delete msg;
	}

}
