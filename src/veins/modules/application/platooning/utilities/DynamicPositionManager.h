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

#ifndef DYNAMICPOSITIONMANAGER_H_
#define DYNAMICPOSITIONMANAGER_H_

#include <map>

class DynamicPositionManager {

    // map from position within the platoon to vehicle id
    typedef std::map<int, int> Platoon;
    // map from platoon id to platoon structure
    typedef std::map<int, Platoon> Platoons;
    // map from vehicle id to own platoon id
    typedef std::map<int, int> VehicleToPlatoon;
    // map from position to vehicle id
    typedef std::map<int, int> Position;
    // map from platoon id to positions
    typedef std::map<int, Position> Positions;

public:
    void addVehicleToPlatoon(const int vehicleId, const int position, const int platoonId);
    void removeVehicleFromPlatoon(const int vehicleId);
    void removeVehicleFromPlatoon(const int vehicleId, const int position, const int platoonId)
    {
        removeVehicleFromPlatoon(vehicleId);
    }
    void printPlatoons();

    static DynamicPositionManager& getInstance();

private:
    DynamicPositionManager()
    {
    }

public:
    Platoons platoons;
    Positions positions;
    VehicleToPlatoon vehToPlatoons;
};

#endif
