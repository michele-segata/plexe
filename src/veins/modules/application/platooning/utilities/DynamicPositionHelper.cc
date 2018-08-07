//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
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

#include <algorithm>

Define_Module(DynamicPositionHelper);

void DynamicPositionHelper::initialize(int stage)
{

    BasePositionHelper::initialize(stage);

    if (stage == 0) {
        myId = getIdFromExternalId(getExternalId());
    }
}

int DynamicPositionHelper::getPosition() const
{
    int platoonId = getPlatoonId();
    return positions.positions.find(platoonId)->second.find(myId)->second;
}

int DynamicPositionHelper::getMemberId(const int position) const
{
    int platoonId = getPlatoonId();
    return positions.platoons.find(platoonId)->second.find(position)->second;
}

int DynamicPositionHelper::getMemberPosition(const int vehicleId) const
{
    int platoonId = getPlatoonId();
    return positions.positions.find(platoonId)->second.find(vehicleId)->second;
}

int DynamicPositionHelper::getLeaderId() const
{
    return getMemberId(0);
}

bool DynamicPositionHelper::isLeader() const
{
    return getPosition() == 0;
}

int DynamicPositionHelper::getFrontId() const
{
    return getMemberId(getPosition() - 1);
}

int DynamicPositionHelper::getPlatoonId() const
{
    auto i = positions.vehToPlatoons.find(myId);
    if (i == positions.vehToPlatoons.end()) return -1;
    int platoonId = i->second;
    return platoonId;
}

int DynamicPositionHelper::getPlatoonLane() const
{
    return 0;
}

bool DynamicPositionHelper::isInSamePlatoon(const int vehicleId) const
{
    auto i = positions.vehToPlatoons.find(vehicleId);
    if (i == positions.vehToPlatoons.end()) return false;
    if (i->second == -1) return false;
    return i->second == getPlatoonId();
}

int DynamicPositionHelper::getPlatoonSize() const
{
    return positions.platoons.find(getPlatoonId())->second.size();
}

const std::vector<int>& DynamicPositionHelper::getPlatoonFormation() const
{
    auto m = positions.platoons.find(getPlatoonId())->second;
    // we do not need to sort the vehicles by their position,
    // since the map<pos, id> is sorted by default by its key (i.e. pos)
    formationCache.resize(m.size());
    std::transform(m.begin(), m.end(), formationCache.begin(), [](const decltype(m)::value_type& p) { return p.second; });
    return formationCache;
}

void DynamicPositionHelper::setPlatoonFormation(const std::vector<int>& formation)
{
    positions.platoons.find(getPlatoonId())->second.clear();
    for (unsigned i = 0; i < formation.size(); i++) {
        addVehicleToPlatoon(formation[i], i, getPlatoonId());
    }
}

void DynamicPositionHelper::addVehicleToPlatoon(int vehicleId, int position, int platoonId)
{
    positions.addVehicleToPlatoon(vehicleId, position, platoonId);
}

void DynamicPositionHelper::removeVehicleFromPlatoon(int vehicleId, int position, int platoonId)
{
    positions.removeVehicleFromPlatoon(vehicleId, position, platoonId);
}

int DynamicPositionHelper::getIdFromExternalId(const std::string externalId)
{
    int dotIndex = externalId.find_last_of('.');
    std::string strId = externalId.substr(dotIndex + 1);
    return strtol(strId.c_str(), 0, 10);
}
