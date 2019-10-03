//
// Copyright (C) 2012-2019 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "plexe/utilities/DynamicPositionManager.h"

#include <algorithm>
#include <iostream>

namespace plexe {

DynamicPositionManager& DynamicPositionManager::getInstance()
{
    static DynamicPositionManager instance;
    return instance;
}

void DynamicPositionManager::addVehicleToPlatoon(const int vehicleId, const int position, const int platoonId)
{
    platoons[platoonId][position] = vehicleId;
    positions[platoonId][vehicleId] = position;
    vehToPlatoons[vehicleId] = platoonId;
}

void DynamicPositionManager::removeVehicleFromPlatoon(const int vehicleId)
{
    auto pId = vehToPlatoons.find(vehicleId);
    if (pId != vehToPlatoons.end()) {
        auto platoon = platoons.find(pId->second);
        int size = platoon->second.size();
        auto pPosition = positions.find(pId->second);
        int pos = pPosition->second.find(vehicleId)->second;
        for (int i = pos; i < size - 1; i++) {
            auto oldPos = platoon->second.find(i + 1);
            platoons[pId->second][i] = oldPos->second;
            positions[pId->second][oldPos->second] = i;
        }
        platoon->second.erase(platoon->second.find(size - 1));
        pPosition->second.erase(pPosition->second.find(vehicleId));
        vehToPlatoons.erase(pId);
    }
}

void DynamicPositionManager::printPlatoons()
{
    for (auto i = platoons.begin(); i != platoons.end(); i++) {
        std::cout << "Platoon " << i->first << ":\n";
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            std::cout << "\tPos " << j->first << ": " << j->second << "\n";
        }
    }
    for (auto i = positions.begin(); i != positions.end(); i++) {
        std::cout << "Platoon " << i->first << ":\n";
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            std::cout << "\tVeh " << j->first << ": " << j->second << "\n";
        }
    }
    for (auto i = vehToPlatoons.begin(); i != vehToPlatoons.end(); i++) {
        std::cout << "Veh " << i->first << ": Platoon " << i->second << "\n";
    }
}

void DynamicPositionManager::setPlatoonInformation(int platoonId, const PlatoonInfo& info)
{
    information[platoonId] = info;
}

PlatoonInfo DynamicPositionManager::getPlatoonInformation(int platoonId) const
{
    PlatoonInfo info;
    info.lane = -1;
    info.speed = -1;
    auto i = information.find(platoonId);
    if (i == information.end()) return info;
    else return i->second;
}

int DynamicPositionManager::getPlatoonId(int vehicleId) const
{
    auto i = vehToPlatoons.find(vehicleId);
    if (i == vehToPlatoons.end()) return -1;
    int platoonId = i->second;
    return platoonId;
}

std::vector<int> DynamicPositionManager::getPlatoonFormation(int vehicleId) const
{
    auto m = platoons.find(getPlatoonId(vehicleId))->second;
    std::vector<int> formation;
    // we do not need to sort the vehicles by their position,
    // since the map<pos, id> is sorted by default by its key (i.e. pos)
    formation.resize(m.size());
    std::transform(m.begin(), m.end(), formation.begin(), [](const decltype(m)::value_type& p) { return p.second; });
    return formation;
}

int DynamicPositionManager::getPosition(int vehicleId) const
{
    int platoonId = getPlatoonId(vehicleId);
    return positions.find(platoonId)->second.find(vehicleId)->second;
}

int DynamicPositionManager::getMemberId(int platoonId, int position) const
{
    return platoons.find(platoonId)->second.find(position)->second;
}

} // namespace plexe
