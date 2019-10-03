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

#include "plexe/utilities/BasePositionHelper.h"

#include <iostream>

using namespace veins;

namespace plexe {

Define_Module(BasePositionHelper);

void BasePositionHelper::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == 0) {
        mobility = veins::TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        myId = getIdFromExternalId(getExternalId());
    }

    if (stage == 1) {
        formation = positions.getPlatoonFormation(myId);
        position = positions.getPosition(myId);
        platoonId = positions.getPlatoonId(myId);
        PlatoonInfo info = positions.getPlatoonInformation(platoonId);
        platoonSpeed = info.speed;
        platoonLane = info.lane;
        setVariablesAfterFormationChange();
    }
}

int BasePositionHelper::getIdFromExternalId(const std::string externalId)
{
    int dotIndex = externalId.find_last_of('.');
    std::string strId = externalId.substr(dotIndex + 1);
    return strtol(strId.c_str(), 0, 10);
}


int BasePositionHelper::numInitStages() const
{
    return 2;
}

void BasePositionHelper::setVariablesAfterFormationChange()
{
    memberToPosition.clear();
    for (int i = 0; i < formation.size(); i++)
        memberToPosition[formation[i]] = i;
    position = getMemberPosition(myId);
    leaderId = formation[0];
    frontId = isLeader() ? -1 : formation[position - 1];
    backId = isLast() ? -1 : formation[position + 1];
}

std::string BasePositionHelper::getExternalId() const
{
    return mobility->getExternalId();
}

int BasePositionHelper::getId() const
{
    return myId;
}

int BasePositionHelper::getPosition() const
{
    return position;
}

int BasePositionHelper::getLeaderId() const
{
    return leaderId;
}

bool BasePositionHelper::isLeader() const
{
    return getLeaderId() == myId;
}

bool BasePositionHelper::isLast() const
{
    return position == formation.size() - 1;
}

int BasePositionHelper::getFrontId() const
{
    if (isLeader())
        return -1;
    else
        return frontId;
}

int BasePositionHelper::getBackId() const
{
    return backId;
}

int BasePositionHelper::getMemberId(const int position) const
{
    if (position < formation.size()) return formation[position];
    else return -1;
}

int BasePositionHelper::getMemberPosition(const int vehicleId) const
{
    auto i = memberToPosition.find(vehicleId);
    if (i == memberToPosition.end()) return -1;
    else return i->second;
}

int BasePositionHelper::getPlatoonId() const
{
    return platoonId;
}

int BasePositionHelper::getPlatoonLane() const
{
    return platoonLane;
}

double BasePositionHelper::getPlatoonSpeed() const
{
    return platoonSpeed;
}

bool BasePositionHelper::isInSamePlatoon(const int vehicleId) const
{
    return getMemberPosition(vehicleId) != -1;
}

int BasePositionHelper::getPlatoonSize() const
{
    return formation.size();
}

void BasePositionHelper::setId(const int id)
{
    myId = id;
}

void BasePositionHelper::setPlatoonId(const int id)
{
    platoonId = id;
}

void BasePositionHelper::setPlatoonLane(const int lane)
{
    platoonLane = lane;
}

void BasePositionHelper::setPlatoonSpeed(double speed)
{
    platoonSpeed = speed;
}

const std::vector<int>& BasePositionHelper::getPlatoonFormation() const
{
    return formation;
}

void BasePositionHelper::setPlatoonFormation(const std::vector<int>& formation)
{
    this->formation = formation;
    setVariablesAfterFormationChange();
}

void BasePositionHelper::dumpVehicleData() const
{
    std::cout << "Vehicle ID: " << myId << "\n";
    std::cout << "\tPlatoon ID      : " << platoonId << "\n";
    std::cout << "\tLeader ID       : " << leaderId << "\n";
    std::cout << "\tFront ID        : " << frontId << "\n";
    std::cout << "\tBack ID         : " << backId << "\n";
    std::cout << "\tPlatoon speed   : " << platoonSpeed << " (m/s)\n";
    std::cout << "\tPlatoon lane    : " << platoonLane << "\n";
    std::cout << "\tPlatoon size    : " << formation.size() << "\n";
    std::cout << "\tStored formation: ";
    for (auto& v : formation)
        std::cout << v << " ";
    std::cout << "\n";
}

} // namespace plexe
