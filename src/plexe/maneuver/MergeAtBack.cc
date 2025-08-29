//
// Copyright (C) 2012-2025 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/maneuver/MergeAtBack.h"
#include "plexe/apps/GeneralPlatooningApp.h"

namespace plexe {

MergeAtBack::MergeAtBack(GeneralPlatooningApp* app)
    : JoinAtBack(app)
    , oldPlatoonId(-1)
    , checkDistance(new cMessage("checkDistance"))
{
}

MergeAtBack::~MergeAtBack()
{
    delete checkDistance;
    checkDistance = nullptr;
}

void MergeAtBack::startManeuver(const void* parameters)
{
    // store the current formation to inform the members when they need to switch platoon id and leader
    oldFormation = positionHelper->getPlatoonFormation();
    // store the old platoon id as well
    oldPlatoonId = positionHelper->getPlatoonId();
    if (initializeJoinManeuver(parameters)) {
        // add additional members if this vehicle is a leader of a platoon about to merge
        std::vector<int> members;
        for (int i = 1; i < positionHelper->getPlatoonFormation().size(); i++)
            members.push_back(positionHelper->getPlatoonFormation()[i]);

        // send merge request to leader
        LOG << positionHelper->getId() << " sending MergePlatoonRequest to platoon with id " << targetPlatoonData->platoonId << " (leader id " << targetPlatoonData->platoonLeader << ")\n";
        MergePlatoonRequest* req = createMergePlatoonRequest(positionHelper->getId(), positionHelper->getExternalId(), targetPlatoonData->platoonId, targetPlatoonData->platoonLeader, traciVehicle->getLaneIndex(), mobility->getPositionAt(simTime()).x, mobility->getPositionAt(simTime()).y, members);
        app->sendUnicast(req, targetPlatoonData->platoonLeader, req->getKind());
    }
}

void MergeAtBack::handleJoinPlatoonRequest(const JoinPlatoonRequest* msg)
{
    // do nothing. This implementation is meant for merging platoons
}

void MergeAtBack::handleMergePlatoonRequest(const MergePlatoonRequest* msg)
{
    if (processJoinRequest(msg)) {
        // add additional vehicles to the new formation to account for the members of the second platoon
        if (msg->getMembersArraySize() != 0) {
            for (unsigned int i = 0; i < msg->getMembersArraySize(); i++) {
                joinerData->newFormation.push_back(msg->getMembers(i));
            }
        }
        LOG << positionHelper->getId() << " sending MoveToPosition to vehicle with id " << msg->getVehicleId() << "\n";
        MoveToPosition* mtp = createMoveToPosition(positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), joinerData->joinerId, positionHelper->getPlatoonSpeed(), positionHelper->getPlatoonLane(), joinerData->newFormation);
        app->sendUnicast(mtp, joinerData->joinerId, mtp->getKind());
    }
}

void MergeAtBack::handleJoinFormation(const JoinFormation* msg)
{
    // when we are allowed to join the platoon, periodically check the distance to the front vehicle
    if (checkDistance->isScheduled()) return;
    app->scheduleAt(simTime() + 0.5, checkDistance);
    JoinAtBack::handleJoinFormation(msg);
}

bool MergeAtBack::handleSelfMsg(cMessage* msg)
{
    if (msg == checkDistance) {
        double distance, relativeSpeed;
        plexeTraciVehicle->getRadarMeasurements(distance, relativeSpeed);
        // we are close enough to the front platoon. tell the followers to change the platoon composition
        if (distance < app->getTargetDistance(targetPlatoonData->platoonSpeed) + 1) {
            for (unsigned int i = 1; i < oldFormation.size(); i++) {
                UpdatePlatoonData* mm = app->createUpdatePlatoonData(positionHelper->getId(), positionHelper->getExternalId(), oldPlatoonId, oldFormation[i], targetPlatoonData->platoonSpeed, targetPlatoonData->platoonLane, targetPlatoonData->newFormation, targetPlatoonData->platoonId);
                app->sendUnicast(mm, oldFormation[i], mm->getKind());
            }
        }
        else {
            app->scheduleAt(simTime() + 0.5, checkDistance);
        }
        return true;
    }
    else {
        return false;
    }
}

// final state for leader
// request update of formation information
void MergeAtBack::handleJoinFormationAck(const JoinFormationAck* msg)
{
    if (app->getPlatoonRole() != PlatoonRole::LEADER) return;
    if (joinManeuverState != JoinManeuverState::L_WAIT_JOINER_TO_JOIN) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getVehicleId() != joinerData->joinerId) return;
    for (unsigned i = 0; i < msg->getNewPlatoonFormationArraySize(); i++) ASSERT(msg->getNewPlatoonFormation(i) == joinerData->newFormation.at(i));

    // the joiner has joined the platoon
    // add the joiner to the list of vehicles in the platoon
    positionHelper->setPlatoonFormation(joinerData->newFormation);

    // send to all vehicles in Platoon
    for (unsigned int i = 1; i < positionHelper->getPlatoonSize(); i++) {
        UpdatePlatoonFormation* dup = app->createUpdatePlatoonFormation(positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), -1, positionHelper->getPlatoonSpeed(), traciVehicle->getLaneIndex(), joinerData->newFormation);
        int dest = positionHelper->getMemberId(i);
        dup->setDestinationId(dest);
        app->sendUnicast(dup, dest, dup->getKind());
    }

    joinManeuverState = JoinManeuverState::IDLE;
    app->setInManeuver(false, nullptr);
}

} // namespace plexe
