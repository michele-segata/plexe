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

#include "veins/modules/application/platooning/utilities/DynamicPositionHelper.h"

Define_Module(DynamicPositionHelper);

void DynamicPositionHelper::initialize(int stage) {

	BasePositionHelper::initialize(stage);

	if (stage == 0) {
		myId = getIdFromExternalId(getExternalId());
	}

}

void DynamicPositionHelper::finish() {
	BasePositionHelper::finish();
}

int DynamicPositionHelper::getPosition() {
	int platoonId = getPlatoonId();
	return positions.positions.find(platoonId)->second.find(myId)->second;
}

int DynamicPositionHelper::getMemberId(int position) {
	int platoonId = getPlatoonId();
	return positions.platoons.find(platoonId)->second.find(position)->second;
}

int DynamicPositionHelper::getMemberPosition(int vehicleId) {
	int platoonId = getPlatoonId();
	return positions.positions.find(platoonId)->second.find(vehicleId)->second;
}

int DynamicPositionHelper::getLeaderId() {
	return getMemberId(0);
}

bool DynamicPositionHelper::isLeader() {
	return getPosition() == 0;
}

int DynamicPositionHelper::getFrontId() {
	return getMemberId(getPosition() - 1);
}

int DynamicPositionHelper::getPlatoonId() {
	return positions.vehToPlatoons.find(myId)->second;
}

int DynamicPositionHelper::getPlatoonLane() {
	return 0;
}

bool DynamicPositionHelper::isInSamePlatoon(int vehicleId) {
	return positions.vehToPlatoons.find(vehicleId)->second == getPlatoonId();
}

int DynamicPositionHelper::getPlatoonSize() {
	return positions.platoons.find(getPlatoonId())->second.size();
}

int DynamicPositionHelper::getIdFromExternalId(std::string externalId) {
	int dotIndex = externalId.find_last_of('.');
	std::string strId = externalId.substr(dotIndex + 1);
	return strtol(strId.c_str(), 0, 10);
}
