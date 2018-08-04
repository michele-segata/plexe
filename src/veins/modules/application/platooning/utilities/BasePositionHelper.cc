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

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

using namespace Veins;

Define_Module(BasePositionHelper);

void BasePositionHelper::initialize(int stage)
{

    BaseApplLayer::initialize(stage);

    if (stage == 0) {
        mobility = Veins::TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        nLanes = par("nLanes").longValue();
        platoonSize = par("platoonSize").longValue();
        nCars = par("nCars").longValue();
        highestId = nCars - 1;
    }
}

std::string BasePositionHelper::getExternalId() const
{
    return mobility->getExternalId();
}

int BasePositionHelper::getId() const
{
    return myId;
}

int BasePositionHelper::getHighestId() const
{
    return highestId;
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
    return -1;
}

int BasePositionHelper::getMemberPosition(const int vehicleId) const
{
    return -1;
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
    return false;
}

int BasePositionHelper::getLanesCount() const
{
    return nLanes;
}

int BasePositionHelper::getPlatoonSize() const
{
    return platoonSize;
}

void BasePositionHelper::setId(const int id)
{
    myId = id;
}

void BasePositionHelper::setHighestId(const int id)
{
    highestId = id;
}

void BasePositionHelper::setPosition(const int position)
{
    this->position = position;
}

void BasePositionHelper::setLeaderId(const int id)
{
    leaderId = id;
}

void BasePositionHelper::setIsLeader(const bool isLeader)
{
    leader = isLeader;
}

void BasePositionHelper::setFrontId(const int id)
{
    frontId = id;
}

void BasePositionHelper::setBackId(const int id)
{
    backId = id;
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

void BasePositionHelper::setLanesCount(const int lanes)
{
    nLanes = lanes;
}

void BasePositionHelper::setPlatoonSize(const int size)
{
    platoonSize = size;
}

const std::vector<int>& BasePositionHelper::getPlatoonFormation() const
{
    throw cRuntimeError("not implemented in base class");
}

void BasePositionHelper::setPlatoonFormation(const std::vector<int>& formation)
{
    throw cRuntimeError("not implemented in base class");
}
