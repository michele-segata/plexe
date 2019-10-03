//
// Copyright (C) 2012-2019 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2019 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "plexe/maneuver/JoinManeuver.h"
#include "plexe/apps/GeneralPlatooningApp.h"

namespace plexe {

JoinManeuver::JoinManeuver(GeneralPlatooningApp* app)
    : Maneuver(app)
{
}

void JoinManeuver::onManeuverMessage(const ManeuverMessage* mm)
{
    if (const JoinPlatoonRequest* msg = dynamic_cast<const JoinPlatoonRequest*>(mm)) {
        handleJoinPlatoonRequest(msg);
    }
    else if (const JoinPlatoonResponse* msg = dynamic_cast<const JoinPlatoonResponse*>(mm)) {
        handleJoinPlatoonResponse(msg);
    }
    else if (const MoveToPosition* msg = dynamic_cast<const MoveToPosition*>(mm)) {
        handleMoveToPosition(msg);
    }
    else if (const MoveToPositionAck* msg = dynamic_cast<const MoveToPositionAck*>(mm)) {
        handleMoveToPositionAck(msg);
    }
    else if (const JoinFormation* msg = dynamic_cast<const JoinFormation*>(mm)) {
        handleJoinFormation(msg);
    }
    else if (const JoinFormationAck* msg = dynamic_cast<const JoinFormationAck*>(mm)) {
        handleJoinFormationAck(msg);
    }
}

JoinPlatoonRequest* JoinManeuver::createJoinPlatoonRequest(int vehicleId, std::string externalId, int platoonId, int destinationId, int currentLaneIndex, double xPos, double yPos)
{
    JoinPlatoonRequest* msg = new JoinPlatoonRequest("JoinPlatoonRequest");
    app->fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setCurrentLaneIndex(currentLaneIndex);
    msg->setXPos(xPos);
    msg->setYPos(yPos);
    return msg;
}

JoinPlatoonResponse* JoinManeuver::createJoinPlatoonResponse(int vehicleId, std::string externalId, int platoonId, int destinationId, bool permitted)
{
    JoinPlatoonResponse* msg = new JoinPlatoonResponse("JoinPlatoonResponse");
    app->fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setPermitted(permitted);
    return msg;
}

MoveToPosition* JoinManeuver::createMoveToPosition(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation)
{
    MoveToPosition* msg = new MoveToPosition("MoveToPosition");
    app->fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setPlatoonSpeed(platoonSpeed);
    msg->setPlatoonLane(platoonLane);
    msg->setNewPlatoonFormationArraySize(newPlatoonFormation.size());
    for (unsigned int i = 0; i < newPlatoonFormation.size(); i++) {
        msg->setNewPlatoonFormation(i, newPlatoonFormation[i]);
    }
    return msg;
}

MoveToPositionAck* JoinManeuver::createMoveToPositionAck(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation)
{
    MoveToPositionAck* msg = new MoveToPositionAck("MoveToPositionAck");
    app->fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setPlatoonSpeed(platoonSpeed);
    msg->setPlatoonLane(platoonLane);
    msg->setNewPlatoonFormationArraySize(newPlatoonFormation.size());
    for (unsigned int i = 0; i < newPlatoonFormation.size(); i++) {
        msg->setNewPlatoonFormation(i, newPlatoonFormation[i]);
    }
    return msg;
}

JoinFormation* JoinManeuver::createJoinFormation(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation)
{
    JoinFormation* msg = new JoinFormation("JoinFormation");
    app->fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setPlatoonSpeed(platoonSpeed);
    msg->setPlatoonLane(platoonLane);
    msg->setNewPlatoonFormationArraySize(newPlatoonFormation.size());
    for (unsigned int i = 0; i < newPlatoonFormation.size(); i++) {
        msg->setNewPlatoonFormation(i, newPlatoonFormation[i]);
    }
    return msg;
}

JoinFormationAck* JoinManeuver::createJoinFormationAck(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation)
{
    JoinFormationAck* msg = new JoinFormationAck("JoinFormationAck");
    app->fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setPlatoonSpeed(platoonSpeed);
    msg->setPlatoonLane(platoonLane);
    msg->setNewPlatoonFormationArraySize(newPlatoonFormation.size());
    for (unsigned int i = 0; i < newPlatoonFormation.size(); i++) {
        msg->setNewPlatoonFormation(i, newPlatoonFormation[i]);
    }
    return msg;
}

} // namespace plexe
