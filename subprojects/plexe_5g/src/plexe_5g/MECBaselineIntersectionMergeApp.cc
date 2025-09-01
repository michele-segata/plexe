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

#include "MECBaselineIntersectionMergeApp.h"

#include "plexe_5g/PlexeInetUtils.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#define LOG EV

namespace plexe {

using namespace inet;
using namespace omnetpp;
using namespace simu5g;

Define_Module(MECBaselineIntersectionMergeApp);

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_SEARCH_RESPONSE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4 + 1)
#define PLATOON_SPEED_SIZE_B (PLATOON_QUERY_SIZE_B + 4)
#define PLATOON_CONTACT_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

MECBaselineIntersectionMergeApp::MECBaselineIntersectionMergeApp()
    : MECBaseApp()
    , firstPlatoon(nullptr)
    , lastPlatoon(nullptr)
    , holdPlatoon(nullptr)
    , traci(nullptr)
{
}

MECBaselineIntersectionMergeApp::~MECBaselineIntersectionMergeApp()
{
    delete firstPlatoon;
    delete lastPlatoon;
    delete holdPlatoon;
}


void MECBaselineIntersectionMergeApp::initialize(int stage)
{
    MECBaseApp::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    safetyDistance = par("safetyDistance");

    //testing
    EV << "MECBaselineIntersectionMergeApp::initialize - Mec application "<< getClassName() << " with mecAppId["<< mecAppId << "] has started!" << endl;

}

void MECBaselineIntersectionMergeApp::handleUEAppMsg(inet::Packet* packet)
{
    // determine its source address/port
    L3Address ueAppAddress = packet->getTag<L3AddressInd>()->getSrcAddress();
    int ueAppPort = packet->getTag<L4PortInd>()->getSrcPort();

    if (const IntersectionRequest* r = PlexeInetUtils::decapsulate<IntersectionRequest>(packet)) {
        onIntersectionRequest(r, ueAppAddress, ueAppPort);
    }

    if (const IntersectionExit* e = PlexeInetUtils::decapsulate<IntersectionExit>(packet)) {
        onIntersectionExit(e, ueAppAddress, ueAppPort);
    }

}

void MECBaselineIntersectionMergeApp::onIntersectionRequest(const IntersectionRequest* req, L3Address ueAddress, int uePort){

    Platoon* p = new Platoon();
    p->platoonId = req->getPlatoonId();
    p->leaderId = req->getLeaderId();
    p->distance = req->getDistance();
    p->length = req->getLength();
    p->speed = req->getSpeed();
    p->roadId = req->getRoadId();
    p->leaderAddress = ueAddress;
    p->leaderPort = uePort;

    if (p->platoonId == 0)
        firstPlatoon = p;
    else if (p->platoonId == 1)
        lastPlatoon = p;
    else
        holdPlatoon = p;

    LOG << "MECBaselineIntersectionMergeApp: received IntersectionRequest from platoon leader " << p->leaderId << "\n";

    if(holdPlatoon && firstPlatoon && lastPlatoon){
        double firstPlatoonEntryTime = firstPlatoon->distance / firstPlatoon->speed;
        double lastPlatoonExitTime = (lastPlatoon->distance + lastPlatoon->length) / lastPlatoon->speed;
        double holdPlatoonEntryTime = holdPlatoon->distance / holdPlatoon->speed;
        double holdPlatoonExitTime = (holdPlatoon->distance + holdPlatoon->length) / holdPlatoon->speed;

        // UE addresses are wrong. they are the ones of the platoon that received the last message!

        // firstPlatoon and lastPlatoon are free to enter the intersection at constanst speed
        sendIntersectionClearance(lastPlatoon->leaderId, 0, 50/3.6, lastPlatoon->leaderAddress, lastPlatoon->leaderPort);
        sendIntersectionClearance(firstPlatoon->leaderId, 0, 50/3.6, firstPlatoon->leaderAddress, firstPlatoon->leaderPort);
        LOG << "MECBaselineIntersectionMergeApp: sending IntersectionClearance to platoon leaders " << firstPlatoon->leaderId << " and " << lastPlatoon->leaderId << "\n";

        // case 1: holdPlatoon can enter the intersection at a safe distance before the first platoon
        if(holdPlatoonExitTime + (safetyDistance / holdPlatoon->speed) < firstPlatoonEntryTime) {
            LOG << "MECBaselineIntersectionMergeApp: sending IntersectionClearance to holding leader " << holdPlatoon->leaderId << "\n";
            sendIntersectionClearance(holdPlatoon->leaderId, 0, 50/3.6, holdPlatoon->leaderAddress, holdPlatoon->leaderPort);
            holdSent = false;
        }

        // case 2: holdPlatoon enters the intersection at a safe distance after the last platoon
        else if(holdPlatoonEntryTime > lastPlatoonExitTime + (safetyDistance / lastPlatoon->speed)) {
            LOG << "MECBaselineIntersectionMergeApp: sending IntersectionClearance to holding leader " << holdPlatoon->leaderId << "\n";
            sendIntersectionClearance(holdPlatoon->leaderId, 0, 50/3.6, holdPlatoon->leaderAddress, holdPlatoon->leaderPort);
            holdSent = false;
        }

        // case 3: holdPlatoon holds the position until the last platoon leaves the intersection
        else {
            LOG << "MECBaselineIntersectionMergeApp: sending IntersectionHold to holding leader " << holdPlatoon->leaderId << "\n";
            sendIntersectionHold(holdPlatoon->leaderId, holdPlatoon->leaderAddress, holdPlatoon->leaderPort);
            holdSent = true;
        }
    }
}

void MECBaselineIntersectionMergeApp::onIntersectionExit(const IntersectionExit* msg, L3Address ueAddress, int uePort){

    LOG << "MECBaselineIntersectionMergeApp: received IntersectionExit from platoon " << msg->getPlatoonId() << "\n";
    if (msg->getPlatoonId() == lastPlatoon->platoonId){
        if (holdSent) {
            LOG << "MECBaselineIntersectionMergeApp: sending IntersectionClearance to holding leader " << holdPlatoon->leaderId << "\n";
            sendIntersectionClearance(holdPlatoon->leaderId, 0, 0, holdPlatoon->leaderAddress, holdPlatoon->leaderPort);
        }
        else {
            LOG << "MECBaselineIntersectionMergeApp: no need to send IntersectionClearance to holding leader. Already sent\n";
        }
    }
}

void MECBaselineIntersectionMergeApp::fillIntersectionClearance(IntersectionClearance* e, double a, double ts){
    e->setAcceleration(a);
    e->setTargetSpeed(ts);
}

void MECBaselineIntersectionMergeApp::sendIntersectionClearance(int destination, double a, double ts, L3Address ueAddress, int uePort){
    IntersectionClearance* e = new IntersectionClearance();
    fillIntersectionClearance(e, a, ts);
    e->setDestinationId(destination);
    e->setByteLength(100);
    sendToUEApp(e, ueAddress, uePort);
}

void MECBaselineIntersectionMergeApp::sendIntersectionHold(int destination, L3Address ueAddress, int uePort){
    IntersectionHold* h = new IntersectionHold();
    h->setDestinationId(destination);
    h->setByteLength(100);
    sendToUEApp(h, ueAddress, uePort);
}

} //namespace

