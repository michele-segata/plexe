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
	return -1;
}

int BasePositionHelper::getLeaderId() {
	return getMemberId(0);
}

bool BasePositionHelper::isLeader() {
	return getLeaderId() == myId;
}

int BasePositionHelper::getFrontId() {
	if (isLeader())
		return -1;
	else
		return getMemberId(getPosition() - 1);
}

int BasePositionHelper::getMemberId(int position) {
	return -1;
}

int BasePositionHelper::getMemberPosition(int vehicleId) {
	return -1;
}

int BasePositionHelper::getPlatoonId() {
	return -1;
}

int BasePositionHelper::getPlatoonLane() {
	return -1;
}

bool BasePositionHelper::isInSamePlatoon(int vehicleId) {
	return -1;
}

int BasePositionHelper::getLanesCount() {
	return nLanes;
}

int BasePositionHelper::getPlatoonSize() {
	return platoonSize;
}
