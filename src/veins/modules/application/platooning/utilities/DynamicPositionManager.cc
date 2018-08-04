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

#include "veins/modules/application/platooning/utilities/DynamicPositionManager.h"

#include <iostream>

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
