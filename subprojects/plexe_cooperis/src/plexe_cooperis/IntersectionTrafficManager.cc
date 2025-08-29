//
// Copyright (C) 2014-2025 Michele Segata <segata@ccs-labs.org>
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

#include "IntersectionTrafficManager.h"

namespace plexe {

Define_Module(IntersectionTrafficManager);

void IntersectionTrafficManager::initialize(int stage)
{

    TraCIBaseTrafficManager::initialize(stage);

    if (stage == 0) {

        platoonInsertTime = SimTime(par("platoonInsertTime").doubleValue());
        platoonInsertDistance = par("platoonInsertDistance").doubleValue();
        platoonInsertHeadway = par("platoonInsertHeadway").doubleValue();
        platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();
        platooningVType = par("platooningVType").stdstringValue();

        leftRightCarSpeed = par("leftRightCarSpeed");
        bottomRightCarSpeed = par("bottomRightCarSpeed");
        leftRightInitialPosition = par("leftRightInitialPosition");
        bottomRightInitialPosition = par("bottomRightInitialPosition");
        routeLeftRight = par("routeLeftRight").stdstringValue();
        routeBottomRight = par("routeBottomRight").stdstringValue();

        insertPlatoonMessage = new cMessage("");
        scheduleAt(platoonInsertTime, insertPlatoonMessage);
    }
}

void IntersectionTrafficManager::scenarioLoaded()
{
    automated.id = findVehicleTypeIndex(platooningVType);
    automated.lane = -1;
    automated.position = 0;
}

void IntersectionTrafficManager::handleSelfMsg(cMessage* msg)
{

    TraCIBaseTrafficManager::handleSelfMsg(msg);

    if (msg == insertPlatoonMessage) {
        insertPlatoons();
    }
}

void IntersectionTrafficManager::insertPlatoons()
{
    automated.speed = leftRightCarSpeed;

    int id = 0;
    VehicleInfo vehicleInfo;
    vehicleInfo.controller = ACC;
    vehicleInfo.id = id;
    vehicleInfo.position = 0;
    vehicleInfo.platoonId = id;
    vehicleInfo.distance = 2;
    vehicleInfo.headway = platoonLeaderHeadway;

    automated.position = leftRightInitialPosition;
    automated.lane = 0;
    automated.vehicleId = id;
    addVehicleToQueue(routeLeftRight, automated);
    positions.addVehicleToPlatoon(id, vehicleInfo);
    PlatoonInfo info;
    info.speed = automated.speed;
    info.lane = automated.lane;
    positions.setPlatoonInformation(vehicleInfo.platoonId, info);


    automated.speed = bottomRightCarSpeed;

    id = 1;
    vehicleInfo.controller = ACC;
    vehicleInfo.id = id;
    vehicleInfo.position = 0;
    vehicleInfo.platoonId = id;
    vehicleInfo.distance = 2;
    vehicleInfo.headway = platoonLeaderHeadway;

    automated.position = bottomRightInitialPosition;
    automated.lane = 0;
    automated.vehicleId = id;
    addVehicleToQueue(routeBottomRight, automated);
    positions.addVehicleToPlatoon(id, vehicleInfo);
    info.speed = automated.speed;
    info.lane = automated.lane;
    positions.setPlatoonInformation(vehicleInfo.platoonId, info);


}

IntersectionTrafficManager::~IntersectionTrafficManager()
{
    cancelAndDelete(insertPlatoonMessage);
    insertPlatoonMessage = nullptr;
}

} // namespace plexe
