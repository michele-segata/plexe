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

#pragma once

#include "omnetpp.h"

#include "MECBaseApp.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe_5g/messages/IntersectionRequest_m.h"
#include "plexe_5g/messages/IntersectionClearance_m.h"
#include "plexe_5g/messages/IntersectionHold_m.h"
#include "plexe_5g/messages/IntersectionExit_m.h"

#include "inet/networklayer/common/L3Address.h"

typedef struct {
    int platoonId;
    int leaderId;
    double distance;
    double length;
    int vehCount;
    double speed;
    std::string roadId;
    double reqTime;
    inet::L3Address leaderAddress;
    int leaderPort;
} Platoon;

namespace plexe {

using namespace std;
using namespace omnetpp;
using namespace simu5g;
using namespace inet;

class MECBaselineIntersectionMergeApp : public MECBaseApp
{

private:

protected:
    virtual void initialize(int stage) override;

    virtual void handleUEAppMsg(inet::Packet* packet) override;
    virtual void handleSelfMsg(cMessage *msg) override {};

    veins::TraCICommandInterface* traci;

    // incoming messages
    virtual void onIntersectionRequest(IntersectionRequest* req, L3Address ueAddress, int uePort);
    virtual void onIntersectionExit(IntersectionExit* msg, L3Address ueAddress, int uePort);

    void fillIntersectionClearance(IntersectionClearance* e, double a, double ts);
    void sendIntersectionClearance(int destination, double a, double ts, L3Address ueAddress, int uePort);
    void sendIntersectionHold(int destination, L3Address ueAddress, int uePort);

    double safetyDistance = 0;

    Platoon* firstPlatoon;
    Platoon* lastPlatoon;
    Platoon* holdPlatoon;
    // keep track of whether a hold or a clearance was sent to the last platoon
    bool holdSent = false;

public:
    MECBaselineIntersectionMergeApp();
    virtual ~MECBaselineIntersectionMergeApp();

};

}
