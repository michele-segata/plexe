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

#ifndef DYNAMICPOSITIONMANAGER_H_
#define DYNAMICPOSITIONMANAGER_H_

#include <map>
#include <vector>

namespace plexe {

// platoon information
typedef struct {
    double speed;
    int lane;
} PlatoonInfo;

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
    // map from platoon id to information
    typedef std::map<int, PlatoonInfo> PlatoonInformation;

public:
    void addVehicleToPlatoon(const int vehicleId, const int position, const int platoonId);
    void removeVehicleFromPlatoon(const int vehicleId);
    void removeVehicleFromPlatoon(const int vehicleId, const int position, const int platoonId)
    {
        removeVehicleFromPlatoon(vehicleId);
    }
    void printPlatoons();
    void setPlatoonInformation(int platoonId, const PlatoonInfo& info);
    PlatoonInfo getPlatoonInformation(int platoonId) const;
    int getPlatoonId(int vehicleId) const;
    std::vector<int> getPlatoonFormation(int vehicleId) const;
    int getPosition(int vehicleId) const;
    int getMemberId(int platoonId, const int position) const;

    static DynamicPositionManager& getInstance();

private:
    DynamicPositionManager()
    {
    }

public:
    Platoons platoons;
    Positions positions;
    VehicleToPlatoon vehToPlatoons;
    PlatoonInformation information;
};

} // namespace plexe

#endif
