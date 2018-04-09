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

int DynamicPositionHelper::getPosition() const {
	int platoonId = getPlatoonId();
	return positions.positions.find(platoonId)->second.find(myId)->second;
}

int DynamicPositionHelper::getMemberId(const int position) const {
	int platoonId = getPlatoonId();
	return positions.platoons.find(platoonId)->second.find(position)->second;
}

int DynamicPositionHelper::getMemberPosition(const int vehicleId) const {
	int platoonId = getPlatoonId();
	return positions.positions.find(platoonId)->second.find(vehicleId)->second;
}

int DynamicPositionHelper::getLeaderId() const {
	return getMemberId(0);
}

bool DynamicPositionHelper::isLeader() const {
	return getPosition() == 0;
}

int DynamicPositionHelper::getFrontId() const {
	return getMemberId(getPosition() - 1);
}

int DynamicPositionHelper::getPlatoonId() const {
	return positions.vehToPlatoons.find(myId)->second;
}

int DynamicPositionHelper::getPlatoonLane() const {
	return 0;
}

bool DynamicPositionHelper::isInSamePlatoon(const int vehicleId) const {
	return positions.vehToPlatoons.find(vehicleId)->second == getPlatoonId();
}

int DynamicPositionHelper::getPlatoonSize() const {
	return positions.platoons.find(getPlatoonId())->second.size();
}

int DynamicPositionHelper::getIdFromExternalId(const std::string externalId) {
	int dotIndex = externalId.find_last_of('.');
	std::string strId = externalId.substr(dotIndex + 1);
	return strtol(strId.c_str(), 0, 10);
}
