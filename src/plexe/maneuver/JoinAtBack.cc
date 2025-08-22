//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/maneuver/JoinAtBack.h"
#include "plexe/apps/GeneralPlatooningApp.h"

namespace plexe {

JoinAtBack::JoinAtBack(GeneralPlatooningApp* app)
    : JoinManeuver(app)
    , joinManeuverState(JoinManeuverState::IDLE)
{
}

bool JoinAtBack::initializeJoinManeuver(const void* parameters)
{
    JoinManeuverParameters* pars = (JoinManeuverParameters*) parameters;
    if (joinManeuverState == JoinManeuverState::IDLE) {
        if (app->isInManeuver()) {
            LOG << positionHelper->getId() << " cannot begin the maneuver because already involved in another one\n";
            return false;
        }

        app->setInManeuver(true, this);
        app->setPlatoonRole(PlatoonRole::JOINER);

        // collect information about target Platoon
        targetPlatoonData.reset(new TargetPlatoonData());
        targetPlatoonData->platoonId = pars->platoonId;
        targetPlatoonData->platoonLeader = pars->leaderId;

        // after successful initialization we are going to send a request and wait for a reply
        joinManeuverState = JoinManeuverState::J_WAIT_REPLY;

        return true;
    }
    else {
        return false;
    }
}

void JoinAtBack::startManeuver(const void* parameters)
{
    if (initializeJoinManeuver(parameters)) {
        // send join request to leader
        LOG << positionHelper->getId() << " sending JoinPlatoonRequesto to platoon with id " << targetPlatoonData->platoonId << " (leader id " << targetPlatoonData->platoonLeader << ")\n";
        JoinPlatoonRequest* req = createJoinPlatoonRequest(positionHelper->getId(), positionHelper->getExternalId(), targetPlatoonData->platoonId, targetPlatoonData->platoonLeader, traciVehicle->getLaneIndex(), mobility->getPositionAt(simTime()).x, mobility->getPositionAt(simTime()).y);
        app->sendUnicast(req, targetPlatoonData->platoonLeader, req->getKind());
    }
}

void JoinAtBack::abortManeuver()
{
    joinManeuverState = JoinManeuverState::IDLE;
}

void JoinAtBack::onPlatoonBeacon(const PlatooningBeacon* pb)
{
    if (joinManeuverState == JoinManeuverState::J_MOVE_IN_POSITION) {
        // check correct role
        ASSERT(app->getPlatoonRole() == PlatoonRole::JOINER);

        // if the message comes from the leader
        if (pb->getVehicleId() == targetPlatoonData->newFormation.at(0)) {
            plexeTraciVehicle->setLeaderVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed());
        }
        // if the message comes from the front vehicle
        int frontPosition = targetPlatoonData->joinIndex - 1;
        int frontId = targetPlatoonData->newFormation.at(frontPosition);
        if (pb->getVehicleId() == frontId) {
            // get front vehicle position
            Coord frontPosition(pb->getPositionX(), pb->getPositionY(), 0);
            // get my position
            veins::TraCICoord traciPosition = mobility->getManager()->getConnection()->omnet2traci(mobility->getPositionAt(simTime()));
            Coord position(traciPosition.x, traciPosition.y);
            // compute distance
            double distance = position.distance(frontPosition) - pb->getLength();
            plexeTraciVehicle->setFrontVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed(), distance);
            // if we are in position, tell the leader about that
            if (distance < app->getTargetDistance(app->getTargetController(), targetPlatoonData->platoonSpeed) + 11) {
                // controller and headway time
                // send move to position response to confirm the parameters
                LOG << positionHelper->getId() << " sending MoveToPositionAck to platoon with id " << targetPlatoonData->platoonId << " (leader id " << targetPlatoonData->platoonLeader << ")\n";
                MoveToPositionAck* ack = createMoveToPositionAck(positionHelper->getId(), positionHelper->getExternalId(), targetPlatoonData->platoonId, targetPlatoonData->platoonLeader, targetPlatoonData->platoonSpeed, targetPlatoonData->platoonLane, targetPlatoonData->newFormation);
                app->sendUnicast(ack, targetPlatoonData->newFormation.at(0), ack->getKind());
                joinManeuverState = JoinManeuverState::J_WAIT_JOIN;
            }
        }
    }
}

void JoinAtBack::onFailedTransmissionAttempt(const ManeuverMessage* mm)
{
    throw cRuntimeError("Impossible to send this packet: %s. Maximum number of unicast retries reached", mm->getName());
}

bool JoinAtBack::processJoinRequest(const JoinPlatoonRequest* msg)
{
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return false;

    if (app->getPlatoonRole() != PlatoonRole::LEADER && app->getPlatoonRole() != PlatoonRole::NONE) return false;

    bool permission = app->isJoinAllowed();

    // send response to the joiner
    LOG << positionHelper->getId() << " sending JoinPlatoonResponse to vehicle with id " << msg->getVehicleId() << " (permission to join: " << (permission ? "permitted" : "not permitted") << ")\n";
    JoinPlatoonResponse* response = createJoinPlatoonResponse(positionHelper->getId(), positionHelper->getExternalId(), msg->getPlatoonId(), msg->getVehicleId(), permission);
    app->sendUnicast(response, msg->getVehicleId(), response->getKind());

    if (!permission) return false;

    app->setInManeuver(true, this);
    app->setPlatoonRole(PlatoonRole::LEADER);

    // disable lane changing during maneuver
    plexeTraciVehicle->setFixedLane(traciVehicle->getLaneIndex());
    positionHelper->setPlatoonLane(traciVehicle->getLaneIndex());

    // save some data. who is joining?
    joinerData.reset(new JoinerData());
    joinerData->from(msg);

    // this was only to grant the request
    // now send the data about the platoon to the joiner
    // add the joiner to the end of the platoon
    joinerData->newFormation = positionHelper->getPlatoonFormation();
    joinerData->newFormation.push_back(joinerData->joinerId);

    // after processing the request we are sending a MoveToPosition message and wait for the joiner
    joinManeuverState = JoinManeuverState::L_WAIT_JOINER_IN_POSITION;
    return true;
}

void JoinAtBack::handleJoinPlatoonRequest(const JoinPlatoonRequest* msg)
{
    if (processJoinRequest(msg)) {
        LOG << positionHelper->getId() << " sending MoveToPosition to vehicle with id " << msg->getVehicleId() << "\n";
        MoveToPosition* mtp = createMoveToPosition(positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), joinerData->joinerId, positionHelper->getPlatoonSpeed(), positionHelper->getPlatoonLane(), joinerData->newFormation);
        app->sendUnicast(mtp, joinerData->joinerId, mtp->getKind());
    }
}

void JoinAtBack::handleMergePlatoonRequest(const MergePlatoonRequest* msg)
{
    // do nothing. This implementation is meant for single joining cars
}

void JoinAtBack::handleJoinPlatoonResponse(const JoinPlatoonResponse* msg)
{
    if (app->getPlatoonRole() != PlatoonRole::JOINER) return;
    if (joinManeuverState != JoinManeuverState::J_WAIT_REPLY) return;
    if (msg->getPlatoonId() != targetPlatoonData->platoonId) return;
    if (msg->getVehicleId() != targetPlatoonData->platoonLeader) return;

    // evaluate permission
    if (msg->getPermitted()) {
        LOG << positionHelper->getId() << " received JoinPlatoonResponse (allowed to join)\n";
        // wait for information about the join maneuver
        joinManeuverState = JoinManeuverState::J_WAIT_INFORMATION;
        // disable lane changing during maneuver
        plexeTraciVehicle->setFixedLane(traciVehicle->getLaneIndex());
    }
    else {
        LOG << positionHelper->getId() << " received JoinPlatoonResponse (not allowed to join)\n";
        // abort maneuver
        joinManeuverState = JoinManeuverState::IDLE;
        app->setPlatoonRole(PlatoonRole::NONE);
        app->setInManeuver(false, nullptr);
    }
}

void JoinAtBack::handleMoveToPosition(const MoveToPosition* msg)
{
    if (app->getPlatoonRole() != PlatoonRole::JOINER) return;
    if (joinManeuverState != JoinManeuverState::J_WAIT_INFORMATION) return;
    if (msg->getPlatoonId() != targetPlatoonData->platoonId) return;
    if (msg->getVehicleId() != targetPlatoonData->platoonLeader) return;

    // the leader told us to move in position, we can start
    // save some data about the platoon
    targetPlatoonData->from(msg);

    // check for correct lane. if not in correct lane, change it
    // if this already is the platoon lane, join at the back (or v.v.)
    // if this is not the plaoon lane, we have to move into longitudinal
    // position
    int currentLane = traciVehicle->getLaneIndex();
    if (currentLane != targetPlatoonData->platoonLane) {
        plexeTraciVehicle->setFixedLane(targetPlatoonData->platoonLane);
    }

    // approaching the platoon
    LOG << positionHelper->getId() << " received MoveToPosition. Increasing speed to approach the platoon\n";

    // activate faked CACC. this way we can approach the front car
    // using data obtained through GPS
    plexeTraciVehicle->setCACCConstantSpacing(15);
    // we have no data so far, so for the moment just initialize
    // with some fake data
    plexeTraciVehicle->setLeaderVehicleFakeData(0, 0, targetPlatoonData->platoonSpeed);
    plexeTraciVehicle->setFrontVehicleFakeData(0, 0, targetPlatoonData->platoonSpeed, 15);
    // set a CC speed higher than the platoon speed to approach it
    plexeTraciVehicle->setCruiseControlDesiredSpeed(targetPlatoonData->platoonSpeed + (30 / 3.6));
    plexeTraciVehicle->setActiveController(FAKED_CACC);

    joinManeuverState = JoinManeuverState::J_MOVE_IN_POSITION;
}

void JoinAtBack::handleMoveToPositionAck(const MoveToPositionAck* msg)
{
    if (app->getPlatoonRole() != PlatoonRole::LEADER) return;
    if (joinManeuverState != JoinManeuverState::L_WAIT_JOINER_IN_POSITION) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getVehicleId() != joinerData->joinerId) return;

    for (unsigned i = 0; i < msg->getNewPlatoonFormationArraySize(); i++) ASSERT(msg->getNewPlatoonFormation(i) == joinerData->newFormation.at(i));

    // the joiner is now in position and is ready to join

    // tell the joiner to join the platoon
    LOG << positionHelper->getId() << " sending JoinFormation to vehicle with id " << joinerData->joinerId << "\n";
    JoinFormation* jf = createJoinFormation(positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), joinerData->joinerId, positionHelper->getPlatoonSpeed(), traciVehicle->getLaneIndex(), joinerData->newFormation);
    app->sendUnicast(jf, joinerData->joinerId, jf->getKind());
    joinManeuverState = JoinManeuverState::L_WAIT_JOINER_TO_JOIN;
}

void JoinAtBack::handleJoinFormation(const JoinFormation* msg)
{
    if (app->getPlatoonRole() != PlatoonRole::JOINER) return;
    if (joinManeuverState != JoinManeuverState::J_WAIT_JOIN) return;
    if (msg->getPlatoonId() != targetPlatoonData->platoonId) return;
    if (msg->getVehicleId() != targetPlatoonData->platoonLeader) return;

    for (unsigned i = 0; i < msg->getNewPlatoonFormationArraySize(); i++) ASSERT(msg->getNewPlatoonFormation(i) == targetPlatoonData->newFormation[i]);

    // we got confirmation from the leader
    // switch from faked CACC to real CACC
    plexeTraciVehicle->setActiveController(app->getTargetController());
    positionHelper->setDistance(app->getStandstillDistance(app->getTargetController()));
    positionHelper->setHeadway(app->getHeadway(app->getTargetController()));
    // set spacing to the target distance to get close to the platoon
    if (positionHelper->getController() == CACC || positionHelper->getController() == FLATBED) plexeTraciVehicle->setCACCConstantSpacing(app->getTargetDistance(targetPlatoonData->platoonSpeed));

    // update platoon information
    positionHelper->setPlatoonId(msg->getPlatoonId());
    positionHelper->setPlatoonLane(targetPlatoonData->platoonLane);
    positionHelper->setPlatoonSpeed(targetPlatoonData->platoonSpeed);
    std::vector<int> formation;
    for (unsigned i = 0; i < msg->getNewPlatoonFormationArraySize(); i++) formation.push_back(msg->getNewPlatoonFormation(i));
    positionHelper->setPlatoonFormation(formation);

    // tell the leader that we're now in the platoon
    LOG << positionHelper->getId() << " received JoinFormation. Sending JoinFormationAck and performing final approach to platoon " << positionHelper->getPlatoonId() << "\n";
    JoinFormationAck* jfa = createJoinFormationAck(positionHelper->getId(), positionHelper->getExternalId(), positionHelper->getPlatoonId(), targetPlatoonData->platoonLeader, positionHelper->getPlatoonSpeed(), traciVehicle->getLaneIndex(), formation);
    app->sendUnicast(jfa, positionHelper->getLeaderId(), jfa->getKind());

    app->setPlatoonRole(PlatoonRole::FOLLOWER);
    joinManeuverState = JoinManeuverState::IDLE;

    app->setInManeuver(false, nullptr);
}

// final state for leader
// request update of formation information
void JoinAtBack::handleJoinFormationAck(const JoinFormationAck* msg)
{
    if (app->getPlatoonRole() != PlatoonRole::LEADER) return;
    if (joinManeuverState != JoinManeuverState::L_WAIT_JOINER_TO_JOIN) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getVehicleId() != joinerData->joinerId) return;
    for (unsigned i = 0; i < msg->getNewPlatoonFormationArraySize(); i++) ASSERT(msg->getNewPlatoonFormation(i) == joinerData->newFormation.at(i));

    // the joiner has joined the platoon
    // add the joiner to the list of vehicles in the platoon
    positionHelper->setPlatoonFormation(joinerData->newFormation);

    LOG << positionHelper->getId() << " received JoinFormationAck. Sending UpdatePlatoonFormation to all members\n";
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
