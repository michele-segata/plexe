//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "MECOvertakeApp.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "nodes/mec/utils/httpUtils/httpUtils.h"
#include "nodes/mec/utils/httpUtils/json.hpp"
#include "nodes/mec/MECPlatform/MECServices/packets/HttpResponseMessage/HttpResponseMessage.h"

#include "plexe_5g/PlexeInetUtils.h"

#define LOG EV

namespace plexe {

using namespace inet;
using namespace omnetpp;
using namespace simu5g;

Define_Module(MECOvertakeApp);

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_SEARCH_RESPONSE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4 + 1)
#define PLATOON_SPEED_SIZE_B (PLATOON_QUERY_SIZE_B + 4)
#define PLATOON_CONTACT_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

const char* MECOvertakeApp::OVERTAKING_STATE_TEXT[] = {
        "WAITING_REQUEST",
        "NOT_STARTED",
        "OVERTAKING_NOWAIT",
        "OVERTAKING_NOWAIT_SLOWDOWN",
        "WAITING_PASSBY",
        "OVERTAKING_AFTER_PASSBY",
        "COMPLETED"
};

const char* MECOvertakeApp::SAFETY_CONDITION_TEXT[] = {
    "OVERTAKE_NO_SLOWDOWN",
    "OVERTAKE_WITH_SLOWDOWN",
    "NO_OVERTAKE"
};

MECOvertakeApp::MECOvertakeApp(): MECBaseApp()
{
    traci = nullptr;

}
MECOvertakeApp::~MECOvertakeApp()
{
}


void MECOvertakeApp::initialize(int stage)
{
    MECBaseApp::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    rho = par("rho");
    epsilon = par("epsilon");
    useCooperativeOvertake = par("useCooperativeOvertake");
    v3 = par("v3");

//    std::string migratorModule = par("migrator");
//    if (migratorModule != "") {
//        std::stringstream migratorPath;
//        migratorPath << "<root>." << migratorModule;
//        migrator = dynamic_cast<Migrator*>(findModuleByPath(migratorPath.str().c_str()));
//        if (!migrator)
//            throw cRuntimeError("MECOvertakeApp: cannot find migrator module at path %s", migratorPath.str().c_str());
//    }

    //testing
    EV << "MECOvertakeApp::initialize - Mec application "<< getClassName() << " with mecAppId["<< mecAppId << "] has started!" << endl;

}

void MECOvertakeApp::finish(){
    recordScalar("usedOvertakingStrategy", usedOvertakingStrategy);
}

double MECOvertakeApp::computeAverageDelay()
{
    double avgDelay = 0;
    for (auto d : delays) {
        avgDelay += d.second;
    }
    return avgDelay / delays.size();
}

void MECOvertakeApp::handleUEAppMsg(inet::Packet* packet)
{
    // determine its source address/port
    L3Address ueAppAddress = packet->getTag<L3AddressInd>()->getSrcAddress();
    int ueAppPort = packet->getTag<L4PortInd>()->getSrcPort();

    // while migration is active, do not process incoming packets to avoid inconsistencies
    // TODO: this can still create inconsistencies if:
    // 1) at time t_0 MEC1 receives a packet, migration is active, packet is ignored and the protocol state is not updated
    // 2) at time t_0+epsilon migration completes
    // 3) at time t_0+2epsilon MEC2 receives the same packet, process it, updates its state
    // at this point the two MEC "mirrored instances" are not synchronized anymore
//    if (migrator && migrator->isMigrating()) {
//        return;
//    }

//    if (migrator) {
//        std::cout << packet->getName() << "\n";
//        auto a = packet->peekAll()->getAllTags<CreationTimeTag>();
//        for (auto t : a) {
//            std::cout << "Peek all tag: " << t.getClassName() << "\n";
//        }
//        SharingTagSet sts = packet->getTags();
//        for (int i = 0; i < sts.getNumTags(); i++) {
//            std::cout << "Tag " << i << " " << sts.getTag(i)->getClassName() << "\n";
//        }
//        if (packet->hasTag<CreationTimeTag>()) {
//            auto creationTimeTag = packet->getTag<CreationTimeTag>();
//            double creationTime = creationTimeTag->getCreationTime().dbl();
//            delays[ueAppAddress] = simTime().dbl() - creationTime;
//            migrator->updateDelay(this, computeAverageDelay());
//        }
//    }

    if (const PlatoonUpdateMessage* update = PlexeInetUtils::decapsulate<PlatoonUpdateMessage>(packet)) {
//        if (migrator) {
//            delays[ueAppAddress] = simTime().dbl() - update->getTime();
//            migrator->updateDelay(this, computeAverageDelay());
//        }
        onPlatoonUpdate(update, ueAppAddress, ueAppPort);
    }

    if (const StartOvertakeRequest* startOvertake = PlexeInetUtils::decapsulate<StartOvertakeRequest>(packet)) {
        onStartOvertakeRequest(startOvertake->getOvertakeSpeed(), startOvertake->getOvertakeAcceleration());
    }

}

void MECOvertakeApp::onStartOvertakeRequest(double os, double oa)
{
    overtakingState = NOT_STARTED;
    incomingOriginalSpeed = incoming.speed;
    overtakeSpeed = os;
    overtakeAcceleration = oa;
}

void MECOvertakeApp::onPlatoonUpdate(const PlatoonUpdateMessage* msg, inet::L3Address ueAppAddress, int ueAppPort)
{
    int platoonId = msg->getPlatoonId();
    PlatoonInfo* platoonInfo;

    switch (platoonId) {
        case OVERTAKING_PLATOON:
            platoonInfo = &overtaking;
            break;
        case OVERTOOK_PLATOON:
            platoonInfo = &overtook;
            break;
        case INCOMING_PLATOON:
            platoonInfo = &incoming;
            break;
        default:
            throw cRuntimeError("MECOvertakeApp::onPlatoonUpdate: unexpected platoon id");
    }

    platoonInfo->platoonId = platoonId;
    platoonInfo->platoonLeader = msg->getVehicleId();
    platoonInfo->leaderAddress = ueAppAddress;
    platoonInfo->leaderPort = ueAppPort;
    platoonInfo->x = msg->getX();
    platoonInfo->y = msg->getY();
    platoonInfo->time = msg->getTime();
    platoonInfo->speed = msg->getSpeed();
    LOG << "MEC Overtake app got update from platoon " << platoonId << ": position x=" << platoonInfo->x << " y=" << platoonInfo->y << " speed=" << platoonInfo->speed << std::endl;

    if (overtakingState != WAITING_REQUEST) {
        computeOvertakingAction();
    }

}

enum SAFETY_CONDITION MECOvertakeApp::computeOvertakeSafety()
{
    //                                  C
    // ------------------------------------
    // B--->       A
    EV << "MECOvertakeApp::computeOvertakeSafety(): compute counter=" << overtakeSafetyComputationCounter++ << std::endl;

    if (!useCooperativeOvertake)
        return NO_OVERTAKE;

    // estimate new positions depending on current time:
    overtaking.x += overtaking.speed * (simTime().dbl() - overtaking.time);
    overtaking.time = simTime().dbl();
    overtook.x += overtook.speed * (simTime().dbl() - overtook.time);
    overtook.time = simTime().dbl();
    incoming.x += -incoming.speed * (simTime().dbl() - incoming.time);
    incoming.time = simTime().dbl();

    // d_s: corresponds to the distance maintained by the ACC
    double d_s = (overtook.speed * 1.2 + 2) + 4;

    // x'_o
    double overtakingSpace = overtook.x - overtaking.x + d_s;
    if (overtakingSpace < 0)
        return OVERTAKE_COMPLETED;

    // compute the time necessary to reach the overtake speed
    double accelerationTime = (overtakeSpeed - overtaking.speed) / overtakeAcceleration;
    // compute v'_o and v'_2
    double vo_prime = overtakeSpeed - overtook.speed;
    double v2_prime = overtaking.speed - overtook.speed;
    // compute the space necessary to reach the overtake speed
    double accelerationSpace = (vo_prime + v2_prime) / 2 * accelerationTime;
    // t_o
    double overtakingTime = 0;

    if (overtakingSpace < accelerationSpace) {
        overtakingTime = -(v2_prime - sqrt(v2_prime * v2_prime + 2 * overtakeAcceleration * overtakingSpace)) / overtakeAcceleration;
    }
    else {
        overtakingTime = accelerationTime + (overtakingSpace - accelerationSpace) / (vo_prime);
    }

    EV << "MECOvertakeApp::computerOvertakeSafety(): overtaking space=" << overtakingSpace << "m, overtakingTime=" << overtakingTime << "s" << std::endl;
    EV << "MECOvertakeApp::computerOvertakeSafety(): projected overtake completion: " << overtaking.x + overtook.speed * overtakingTime + overtakingSpace << "\n";


    // check for safety
    // incoming.speed is with - sign in front because in the math it is assumed to be negative, but in the simulator it is a positive speed
    // x'_3
    double initPosition = (incoming.x - overtook.x);
    // v'_3 * t_o
    double spaceRhov3 = (-incoming.speed - overtook.speed) * overtakingTime;
    if (d_s + epsilon < initPosition + spaceRhov3 && (overtakingState == NOT_STARTED || overtakingState == OVERTAKING_NOWAIT)) {
        EV << "MECOvertakeApp::computerOvertakeSafety(): overtake can be done without slowdown" << std::endl;
        // the maneuver can be done without slowing down incoming traffic
        EV << "MECOvertakeApp::computerOvertakeSafety(): projected incoming position: " << incoming.x - incoming.speed * overtakingTime << "\n";
        return OVERTAKE_NO_SLOWDOWN;
    }
    else {
        // assume slowdown deceleration to be the same as overtake acceleration
        // time necessary to decelerate down to v3 * rho
        double t_d = (incoming.speed - v3 * rho) / overtakeAcceleration;
        // space necessary for decelerating down to v3 * rho
        double x_decel = ((-incoming.speed - overtook.speed) + (-v3 * rho - overtook.speed)) / 2 * t_d;
        // space traveled at speed v3 * rho
        spaceRhov3 = (-rho * v3 - overtook.speed) * (overtakingTime - t_d);
        if (d_s + epsilon < initPosition + x_decel + spaceRhov3) {
            EV << "MECOvertakeApp::computerOvertakeSafety(): overtake can be done with slowdown" << std::endl;
            EV << "MECOvertakeApp::computerOvertakeSafety(): projected incoming position: " << incoming.x + x_decel - rho * v3 * overtakingTime << "\n";
            // the maneuver can be done but incoming traffic needs to slow down
            return OVERTAKE_WITH_SLOWDOWN;
        }
        else {
            EV << "MECOvertakeApp::computerOvertakeSafety(): overtake cannot be done" << std::endl;
            EV << "MECOvertakeApp::computerOvertakeSafety(): projected incoming position: " << incoming.x + x_decel - rho * v3 * overtakingTime << "\n";
            // incoming traffic should slow down too much
            return NO_OVERTAKE;
        }
    }
}

void MECOvertakeApp::computeOvertakingAction()
{

    EV << "MECOvertakeApp::computeOvertakingAction(): current state = " << OVERTAKING_STATE_TEXT[overtakingState] << std::endl;

    double d_s = (overtook.speed * 1.2 + 2);

    if (overtakingState == NOT_STARTED) {
        switch (computeOvertakeSafety()) {
        case OVERTAKE_NO_SLOWDOWN:
            sendChangeLaneCommand(overtaking, 1);
            overtakingState = OVERTAKING_NOWAIT;
            usedOvertakingStrategy = OVERTAKE_WITHOUT_SLOWDOWN;
            break;
        case OVERTAKE_WITH_SLOWDOWN:
            sendChangeLaneCommand(overtaking, 1);
            overtakingState = OVERTAKING_NOWAIT_SLOWDOWN;
            usedOvertakingStrategy = OVERTAKE_WITH_INCOMING_SLOWDOWN;
            break;
        case NO_OVERTAKE:
            overtakingState = WAITING_PASSBY;
            usedOvertakingStrategy = OVERTAKE_AFTER_PASSBY;
            break;
        case OVERTAKE_COMPLETED:
            throw cRuntimeError("Overtake is computed as completed but it has yet not started");
            break;
        }
        EV << "MECOvertakeApp::computeOvertakingAction(): new state = " << OVERTAKING_STATE_TEXT[overtakingState] << std::endl;
    }

    if (overtakingState == OVERTAKING_NOWAIT) {
        switch (computeOvertakeSafety()) {
        case OVERTAKE_COMPLETED:
            break;
        case OVERTAKE_WITH_SLOWDOWN:
            overtakingState = OVERTAKING_NOWAIT_SLOWDOWN;
            usedOvertakingStrategy = OVERTAKE_WITH_INCOMING_SLOWDOWN;
            break;
        case NO_OVERTAKE:
            EV << "MECOvertakeApp::computeOvertakingAction(): After committing to overtake, safety conditions are not met anymore!!!" << std::endl;
            usedOvertakingStrategy = OVERTAKE_BECAME_UNFEASIBLE;
            endSimulation();
            break;
        default:
            break;
        }
        EV << "MECOvertakeApp::computeOvertakingAction(): new state = " << OVERTAKING_STATE_TEXT[overtakingState] << std::endl;

        if (overtakingState == OVERTAKING_NOWAIT) {
            // continue telling the platoon to speed up
            sendSpeedCommand(overtaking, overtakeSpeed);
            if (overtaking.x - overtook.x - 4 > d_s) {
                // the maneuver is completed
                sendChangeLaneCommand(overtaking, -1);
                overtakingState = COMPLETED;
            }
        }
    }

    if (overtakingState == OVERTAKING_NOWAIT_SLOWDOWN) {
        switch (computeOvertakeSafety()) {
        case OVERTAKE_COMPLETED:
            break;
        case NO_OVERTAKE:
            EV << "MECOvertakeApp::computeOvertakingAction(): After committing to overtake, safety conditions are not met anymore!!!" << std::endl;
            usedOvertakingStrategy = OVERTAKE_BECAME_UNFEASIBLE;
            endSimulation();
            break;
        default:
            break;
        }
        sendSpeedCommand(incoming, rho * incomingOriginalSpeed);
        sendSpeedCommand(overtaking, overtakeSpeed);
        if (overtaking.x - overtook.x - 4 > d_s) {
            // the maneuver is completed
            sendSpeedCommand(incoming, incomingOriginalSpeed);
            sendChangeLaneCommand(overtaking, -1);
            overtakingState = COMPLETED;
            EV << "MECOvertakeApp::computeOvertakingAction(): new state = " << OVERTAKING_STATE_TEXT[overtakingState] << std::endl;
        }
    }

    if (overtakingState == WAITING_PASSBY) {
        // must measure 2 car lenghts as in opposite directions, plus a safety margin
        if (overtaking.x > incoming.x + 4 * 2 + 4) {
            overtakingState = OVERTAKING_AFTER_PASSBY;
            EV << "MECOvertakeApp::computeOvertakingAction(): new state = " << OVERTAKING_STATE_TEXT[overtakingState] << std::endl;
        }
    }

    if (overtakingState == OVERTAKING_AFTER_PASSBY) {
        sendChangeLaneCommand(overtaking, 1);
        sendSpeedCommand(overtaking, overtakeSpeed);
        if (overtaking.x - overtook.x - 4 > d_s) {
            // the maneuver is completed
            sendChangeLaneCommand(overtaking, -1);
            overtakingState = COMPLETED;
            EV << "MECOvertakeApp::computeOvertakingAction(): new state = " << OVERTAKING_STATE_TEXT[overtakingState] << std::endl;
        }
    }

}

void MECOvertakeApp::sendToUEApp(cPacket* packet, inet::L3Address ueAddress, int uePort)
{
//    if (!migrator || migrator->canTransmit(this))
        MECBaseApp::sendToUEApp(packet, ueAddress, uePort);
//    else
        //delete packet;
}

void MECOvertakeApp::sendSpeedCommand(const PlatoonInfo& platoon, double speed)
{
    LOG << "Sending speed command to platoon number " << platoon.platoonId << ": set speed to " << speed << "m/s\n";
    PlatoonSpeedCommand* msg = new PlatoonSpeedCommand("platoonSpeedCommand");
    populateResponse(*msg, platoon);
    msg->setSpeed(speed);
    msg->setByteLength(PLATOON_SPEED_SIZE_B);
    sendToUEApp(msg, platoon.leaderAddress, platoon.leaderPort);
}

void MECOvertakeApp::sendChangeLaneCommand(const PlatoonInfo& platoon, int laneChangeRelative)
{
    LOG << "Sending change lane command to platoon number " << platoon.platoonId << ": change lane: " << laneChangeRelative << "m/s\n";
    PlatoonChangeLaneCommand* msg = new PlatoonChangeLaneCommand("platoonLaneChangeCommand");
    populateResponse(*msg, platoon);
    msg->setChangeLane(laneChangeRelative);
    msg->setByteLength(PLATOON_SPEED_SIZE_B);
    sendToUEApp(msg, platoon.leaderAddress, platoon.leaderPort);
}

void MECOvertakeApp::populateResponse(PlatoonTAQuery& msg, const PlatoonInfo& destination)
{
    msg.setPlatoonId(destination.platoonId);
    msg.setVehicleId(destination.platoonLeader);
}

} //namespace

