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

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

Define_Module(BasePositionHelper);

void BasePositionHelper::initialize(int stage) {

	BaseApplLayer::initialize(stage);

	if (stage == 0) {
		mobility = Veins::TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();
		nLanes = par("nLanes").longValue();
		platoonSize = par("platoonSize").longValue();
		nCars = par("nCars").longValue();
		highestId = nCars - 1;
	}

}

void BasePositionHelper::finish() {
	BaseApplLayer::finish();
}

std::string BasePositionHelper::getExternalId() {
	return mobility->getExternalId();
}

int BasePositionHelper::getId() {
	return myId;
}

int BasePositionHelper::getHighestId() {
	return highestId;
}

int BasePositionHelper::getPosition() {
	return position;
}

int BasePositionHelper::getLeaderId() {
	return leaderId;
}

bool BasePositionHelper::isLeader() {
	return getLeaderId() == myId;
}

int BasePositionHelper::getFrontId() {
	if (isLeader())
		return -1;
	else
		return frontId;
}

int BasePositionHelper::getMemberId(int position) {
	return -1;
}

int BasePositionHelper::getMemberPosition(int vehicleId) {
	return -1;
}

int BasePositionHelper::getPlatoonId() {
	return platoonId;
}

int BasePositionHelper::getPlatoonLane() {
	return platoonLane;
}

bool BasePositionHelper::isInSamePlatoon(int vehicleId) {
	return false;
}

int BasePositionHelper::getLanesCount() {
	return nLanes;
}

int BasePositionHelper::getPlatoonSize() {
	return platoonSize;
}

void BasePositionHelper::setId(int id) {
	myId = id;
}

void BasePositionHelper::setHighestId(int id) {
	highestId = id;
}

void BasePositionHelper::setPosition(int position) {
	this->position = position;
}

void BasePositionHelper::setLeaderId(int id) {
	leaderId = id;
}

void BasePositionHelper::setIsLeader(bool isLeader) {
	leader = isLeader;
}

void BasePositionHelper::setFrontId(int id) {
	frontId = id;
}

void BasePositionHelper::setPlatoonId(int id) {
	platoonId = id;
}

void BasePositionHelper::setPlatoonLane(int lane) {
	platoonLane = lane;
}

void BasePositionHelper::setLanesCount(int lanes) {
	nLanes = lanes;
}

void BasePositionHelper::setPlatoonSize(int size) {
	platoonSize = size;
}
