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

#ifndef JOINMANEUVER_H_
#define JOINMANEUVER_H_

#include "plexe/maneuver/Maneuver.h"
#include "plexe/messages/JoinFormationAck_m.h"
#include "plexe/messages/JoinFormation_m.h"
#include "plexe/messages/JoinPlatoonRequest_m.h"
#include "plexe/messages/JoinPlatoonResponse_m.h"
#include "plexe/messages/ManeuverMessage_m.h"
#include "plexe/messages/MoveToPositionAck_m.h"
#include "plexe/messages/MoveToPosition_m.h"

namespace plexe {

struct JoinManeuverParameters {
    int platoonId;
    int leaderId;
    int position;
};

class JoinManeuver : public Maneuver {

public:
    /**
     * Constructor
     *
     * @param app pointer to the generic application used to fetch parameters and inform it about a concluded maneuver
     */
    JoinManeuver(GeneralPlatooningApp* app);
    virtual ~JoinManeuver(){};

    virtual void onManeuverMessage(const ManeuverMessage* mm) override;

protected:
    /**
     * Creates a JoinPlatoonRequest message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int currentLaneIndex the index of the current lane of the vehicle
     */
    JoinPlatoonRequest* createJoinPlatoonRequest(int vehicleId, std::string externalId, int platoonId, int destinationId, int currentLaneIndex, double xPos, double yPos);

    /**
     * Creates a JoinPlatoonResponse message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param bool permitted whether the join maneuver is permitted
     */
    JoinPlatoonResponse* createJoinPlatoonResponse(int vehicleId, std::string externalId, int platoonId, int destinationId, bool permitted);

    /**
     * Creates a MoveToPosition message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int platoonSpeed the speed of the platoon
     * @param int platoonLane the id of the lane of the platoon
     * @param std::vector<int> newPlatoonFormation the platoon formation after
     * the join maneuver
     */
    MoveToPosition* createMoveToPosition(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation);

    /**
     * Creates a MoveToPositionAck message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int platoonSpeed the speed of the platoon
     * @param int platoonLane the id of the lane of the platoon
     * @param std::vector<int> newPlatoonFormation the platoon formation after
     * the join maneuver
     */
    MoveToPositionAck* createMoveToPositionAck(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation);

    /**
     * Creates a JoinFormation message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int platoonSpeed the speed of the platoon
     * @param int platoonLane the id of the lane of the platoon
     * @param std::vector<int> newPlatoonFormation the platoon formation after
     * the join maneuver
     */
    JoinFormation* createJoinFormation(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation);

    /**
     * Creates a JoinFormationAck message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int platoonSpeed the speed of the platoon
     * @param int platoonLane the id of the lane of the platoon
     * @param std::vector<int> newPlatoonFormation the platoon formation after
     * the join maneuver
     */
    JoinFormationAck* createJoinFormationAck(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& newPlatoonFormation);

    /**
     * Handles a JoinPlatoonRequest in the context of this application
     *
     * @param JoinPlatoonRequest msg to handle
     */
    virtual void handleJoinPlatoonRequest(const JoinPlatoonRequest* msg) = 0;

    /**
     * Handles a JoinPlatoonResponse in the context of this application
     *
     * @param JoinPlatoonResponse msg to handle
     */
    virtual void handleJoinPlatoonResponse(const JoinPlatoonResponse* msg) = 0;

    /**
     * Handles a MoveToPosition in the context of this application
     *
     * @param MoveToPosition msg to handle
     */
    virtual void handleMoveToPosition(const MoveToPosition* msg) = 0;

    /**
     * Handles a MoveToPositionAck in the context of this application
     *
     * @param MoveToPositionAck msg to handle
     */
    virtual void handleMoveToPositionAck(const MoveToPositionAck* msg) = 0;

    /**
     * Handles a JoinFormation in the context of this application
     *
     * @param JoinFormation msg to handle
     */
    virtual void handleJoinFormation(const JoinFormation* msg) = 0;

    /**
     * Handles a JoinFormationAck in the context of this application
     *
     * @param JoinFormationAck msg to handle
     */
    virtual void handleJoinFormationAck(const JoinFormationAck* msg) = 0;
};

} // namespace plexe

#endif
