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

#include "MECMatlabIntersectionMergeApp.h"

#include "plexe_5g/PlexeInetUtils.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace plexe {

using namespace inet;
using namespace omnetpp;
using namespace simu5g;

Define_Module(MECMatlabIntersectionMergeApp);

MECMatlabIntersectionMergeApp::MECMatlabIntersectionMergeApp() :
        MECBaselineIntersectionMergeApp()

{
}

MECMatlabIntersectionMergeApp::~MECMatlabIntersectionMergeApp()
{

}

void MECMatlabIntersectionMergeApp::initialize(int stage)
{
    MECBaselineIntersectionMergeApp::initialize(stage);

    // avoid multiple initializations
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    matlab = veins::FindModule<IntersectionMatlab*>::findGlobalModule();
    if (!matlab)
        throw cRuntimeError("Cannot find matlab");

    platoonNum = par("platoonNum");
    maxAcc = par("maxAcc");
    maxSpeed = par("maxSpeed");

    EV << "MECMatlabIntersectionMergeApp::initialize - Mec application " << getClassName() << " with mecAppId[" << mecAppId << "] has started!" << endl;

}

void MECMatlabIntersectionMergeApp::handleUEAppMsg(inet::Packet* packet)
{
    // determine its source address/port
    L3Address ueAppAddress = packet->getTag<L3AddressInd>()->getSrcAddress();
    int ueAppPort = packet->getTag<L4PortInd>()->getSrcPort();

    if (IntersectionUpdate* r = PlexeInetUtils::decapsulate<IntersectionUpdate>(packet)) {
        onIntersectionUpdate(r, ueAppAddress, ueAppPort);
        delete r;
    }
    else {
        MECBaselineIntersectionMergeApp::handleUEAppMsg(packet);
    }

}

void MECMatlabIntersectionMergeApp::onIntersectionRequest(IntersectionRequest *req, L3Address ueAddress, int uePort)
{
    if (platoonMap.find(req->getPlatoonId()) == platoonMap.end()) {
        // new platoon approaching
        Platoon *p = new Platoon();
        p->platoonId = req->getPlatoonId();
        p->leaderId = req->getLeaderId();
        p->distance = req->getDistance();
        p->speed = req->getSpeed();
        p->length = req->getLength();
        p->vehCount = req->getVehCount();
        p->roadId = req->getRoadId();
        p->reqTime = req->getReqTime();
        p->leaderAddress = ueAddress;
        p->leaderPort = uePort;

        platoonMap.insert(std::make_pair(p->platoonId, p));
    }

    printf("MECMatlabIntersectionMergeApp: got intersection request from %d. Speed=%+5.2f, dist=%+5.2f\n", req->getPlatoonId(), req->getSpeed(), req->getDistance());

    if (platoonMap.size() == platoonNum) {
        // update platoons position
        double now = simTime().dbl();

        for (auto p : platoonMap) {
            // TODO: check that this is actually modifying the field
            printf("MECMatlabIntersectionMergeApp: updating distance of platoon %d. Speed=%+5.2f, dist=%+5.2f, delta_t=%+5.2f", p.second->platoonId, p.second->speed, p.second->distance, (now - p.second->reqTime));
            p.second->distance = p.second->distance - (p.second->speed * (now - p.second->reqTime));
            printf(", new distance=%+5.2f\n", p.second->distance);
        }

        // create ds for matlab
        MatlabPlatoons mPlatoons;
        for (auto const& [id, p] : platoonMap) {
            MatlabPlatoon *mp = new MatlabPlatoon();
            mp->distance = p->distance;
            mp->length = p->length;
            mp->roadId = p->roadId;
            mp->speed = p->speed;
            mp->vehCount = p->vehCount;
            mPlatoons.push_back(mp);
        }

        MatlabResult res = matlab->intersection(&mPlatoons, safetyDistance, maxAcc, maxSpeed);

        for (int i = 0; i < platoonMap.size(); i++) {
            Platoon *p = platoonMap.at(i);
            sendIntersectionClearance(p->leaderId, res[i].first, res[i].second, p->leaderAddress, p->leaderPort);
        }
    }
}

void MECMatlabIntersectionMergeApp::onIntersectionUpdate(IntersectionUpdate *req, L3Address ueAddress, int uePort)
{

    auto platoon = platoonMap.find(req->getPlatoonId());
    if (platoon != platoonMap.end()) {
        Platoon* p = platoon->second;
        p->platoonId = req->getPlatoonId();
        p->leaderId = req->getLeaderId();
        p->distance = req->getDistance();
        p->speed = req->getSpeed();
        p->length = req->getLength();
        p->vehCount = req->getVehCount();
        p->roadId = req->getRoadId();
        p->reqTime = req->getReqTime();
    }

    printf("MECMatlabIntersectionMergeApp: got intersection update from %d. Speed=%+5.2f, dist=%+5.2f\n", req->getPlatoonId(), req->getSpeed(), req->getDistance());

    if (platoonMap.size() != platoonNum)
        return;

    // should we compute the optimization and send the updated accelerations?
    // not if we are too close/past the intersection
    bool computeAndSend = true;

    // update platoons position
    double now = simTime().dbl();
    for (auto p : platoonMap) {
        // TODO: check that this is actually modifying the field
        printf("MECMatlabIntersectionMergeApp: updating distance of platoon %d. Speed=%+5.2f, dist=%+5.2f, delta_t=%+5.2f", p.second->platoonId, p.second->speed, p.second->distance, (now - p.second->reqTime));
        p.second->distance = p.second->distance - (p.second->speed * (now - p.second->reqTime));
        // as we are updating the estimated distance we should also update the req time
        p.second->reqTime = now;
        printf(", new distance=%+5.2f\n", p.second->distance);

        if (p.second->distance <= 0 || p.second->distance > 1e9) {
            printf("Distance to intersection is too close. Optimization will not be run\n");
            computeAndSend = false;
        }
    }

    if (!computeAndSend)
        return;

    // create ds for matlab
    MatlabPlatoons mPlatoons;
    for (auto const& [id, p] : platoonMap) {
        MatlabPlatoon *mp = new MatlabPlatoon();
        mp->distance = p->distance;
        mp->length = p->length;
        mp->roadId = p->roadId;
        mp->speed = p->speed;
        mp->vehCount = p->vehCount;
        mPlatoons.push_back(mp);
    }

    MatlabResult res = matlab->intersection(&mPlatoons, safetyDistance, maxAcc, maxSpeed);

    for (int i = 0; i < platoonMap.size(); i++) {
        Platoon *p = platoonMap.at(i);
        sendIntersectionClearanceUpdate(p->leaderId, res[i].first, res[i].second, p->leaderAddress, p->leaderPort);
    }

}

void MECMatlabIntersectionMergeApp::sendIntersectionClearanceUpdate(int destination, double a, double ts, L3Address ueAddress, int uePort)
{
    IntersectionClearanceUpdate* e = new IntersectionClearanceUpdate();
    fillIntersectionClearance(e, a, ts);
    e->setDestinationId(destination);
    e->setByteLength(100);
    sendToUEApp(e, ueAddress, uePort);
}

void MECMatlabIntersectionMergeApp::onIntersectionExit(IntersectionExit *msg, L3Address ueAddress, int uePort)
{

}

} //namespace

