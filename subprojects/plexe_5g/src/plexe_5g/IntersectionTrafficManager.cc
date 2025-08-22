//
// Copyright (C) 2014-2022 Michele Segata <segata@ccs-labs.org>
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
        platoonInsertSpeed = par("platoonInsertSpeed").doubleValue();
        platoonInsertDistance = par("platoonInsertDistance").doubleValue();
        platoonInsertHeadway = par("platoonInsertHeadway").doubleValue();
        platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();
        platooningVType = par("platooningVType").stdstringValue();
        shockwaveVType = par("shockwaveVType").stdstringValue();
        injectShockwaveCars = par("injectShockwaveCars");

        platoonSizeA = par("platoonSizeA");
        platoonSizeB = par("platoonSizeB");
        platoonSizeC = par("platoonSizeC");
        initialPositionDeltaA = par("initialPositionDeltaA");
        initialPositionDeltaB = par("initialPositionDeltaB");
        initialPositionDeltaC = par("initialPositionDeltaC");
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
    automated.speed = platoonInsertSpeed / 3.6;
    shockwave.id = findVehicleTypeIndex(shockwaveVType);
    shockwave.lane = -1;
    shockwave.position = 0;
    shockwave.speed = platoonInsertSpeed / 3.6;
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

    // compute intervehicle distance
    double distance = platoonInsertSpeed / 3.6 * platoonInsertHeadway + platoonInsertDistance;
    // length of 1 platoon
    double platoonLengthA = platoonSizeA * 4 + (platoonSizeA - 1) * distance;
    double platoonLengthB = platoonSizeB * 4 + (platoonSizeB - 1) * distance;
    double platoonLengthC = platoonSizeC * 4 + (platoonSizeC - 1) * distance;
    // inter-platoon distance
    double platoonDistance = platoonInsertSpeed / 3.6 * platoonLeaderHeadway + 2;
    // total length for left to right route
    double totalLength = platoonLengthA + platoonLengthB + initialPositionDeltaA + initialPositionDeltaB + platoonDistance;

    // first insert platoons A and B
    double currentRoadPosition = totalLength;
    int currentVehicleId = 0;

    if (injectShockwaveCars) {
        shockwave.position = currentRoadPosition + platoonDistance + 4;
        shockwave.lane = 0;
        shockwave.vehicleId = 0;
        addVehicleToQueue(routeLeftRight, shockwave);
    }

    for (int i = 0; i < platoonSizeA; i++) {
        VehicleInfo vehicleInfo;
        vehicleInfo.controller = i == 0 ? ACC : controller;
        vehicleInfo.id = currentVehicleId;
        vehicleInfo.position = i;
        vehicleInfo.platoonId = 0;
        vehicleInfo.distance = i == 0 ? 2 : platoonInsertDistance;
        vehicleInfo.headway = i == 0 ? platoonLeaderHeadway : platoonInsertHeadway;

        automated.position = currentRoadPosition - i * (distance + 4);
        automated.lane = 0;
        automated.vehicleId = currentVehicleId;
        addVehicleToQueue(routeLeftRight, automated);
        positions.addVehicleToPlatoon(currentVehicleId, vehicleInfo);
        currentVehicleId++;
        if (i == 0) {
            PlatoonInfo info;
            info.speed = automated.speed;
            info.lane = automated.lane;
            positions.setPlatoonInformation(vehicleInfo.platoonId, info);
        }
    }

    currentRoadPosition = currentRoadPosition - platoonLengthA - initialPositionDeltaA - platoonDistance;

    for (int i = 0; i < platoonSizeB; i++) {
        VehicleInfo vehicleInfo;
        vehicleInfo.controller = i == 0 ? ACC : controller;
        vehicleInfo.id = currentVehicleId;
        vehicleInfo.position = i;
        vehicleInfo.platoonId = 1;
        vehicleInfo.distance = i == 0 ? 2 : platoonInsertDistance;
        vehicleInfo.headway = i == 0 ? platoonLeaderHeadway : platoonInsertHeadway;

        automated.position = currentRoadPosition - i * (distance + 4);
        automated.lane = 0;
        automated.vehicleId = currentVehicleId;
        addVehicleToQueue(routeLeftRight, automated);
        positions.addVehicleToPlatoon(currentVehicleId, vehicleInfo);
        currentVehicleId++;
        if (i == 0) {
            PlatoonInfo info;
            info.speed = automated.speed;
            info.lane = automated.lane;
            positions.setPlatoonInformation(vehicleInfo.platoonId, info);
        }
    }

    // then add platoon C
    currentRoadPosition = platoonLengthC + initialPositionDeltaC;
    for (int i = 0; i < platoonSizeC; i++) {
        VehicleInfo vehicleInfo;
        vehicleInfo.controller = i == 0 ? ACC : controller;
        vehicleInfo.id = currentVehicleId;
        vehicleInfo.position = i;
        vehicleInfo.platoonId = 2;
        vehicleInfo.distance = i == 0 ? 2 : platoonInsertDistance;
        vehicleInfo.headway = i == 0 ? platoonLeaderHeadway : platoonInsertHeadway;

        automated.position = currentRoadPosition - i * (distance + 4);
        automated.lane = 0;
        automated.vehicleId = currentVehicleId;
        addVehicleToQueue(routeBottomRight, automated);
        positions.addVehicleToPlatoon(currentVehicleId, vehicleInfo);
        currentVehicleId++;
        if (i == 0) {
            PlatoonInfo info;
            info.speed = automated.speed;
            info.lane = automated.lane;
            positions.setPlatoonInformation(vehicleInfo.platoonId, info);
        }
    }

}

IntersectionTrafficManager::~IntersectionTrafficManager()
{
    cancelAndDelete(insertPlatoonMessage);
    insertPlatoonMessage = nullptr;
}

} // namespace plexe
