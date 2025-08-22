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

#include "MECBaselineIntersectionMergeApp.h"

#include "plexe_5g/messages/IntersectionUpdate_m.h"
#include "plexe_5g/messages/IntersectionClearanceUpdate_m.h"
#include "plexe_5g/IntersectionMatlab.h"

namespace plexe {

using namespace std;
using namespace omnetpp;
using namespace simu5g;
using namespace inet;

class MECMatlabIntersectionMergeApp : public MECBaselineIntersectionMergeApp
{

protected:
    virtual void initialize(int stage) override;

    virtual void handleUEAppMsg(inet::Packet* packet) override;

    virtual void onIntersectionRequest(IntersectionRequest* req, L3Address ueAddress, int uePort) override;
    virtual void onIntersectionUpdate(IntersectionUpdate *req, L3Address ueAddress, int uePort);
    virtual void onIntersectionExit(IntersectionExit* msg, L3Address ueAddress, int uePort) override;

    void sendIntersectionClearanceUpdate(int destination, double a, double ts, L3Address ueAddress, int uePort);

    typedef std::map<int, Platoon*> PlatoonMap;
    IntersectionMatlab* matlab;
    PlatoonMap platoonMap;
    int platoonNum;
    double maxAcc;
    double maxSpeed;

public:
    MECMatlabIntersectionMergeApp();
    virtual ~MECMatlabIntersectionMergeApp();

};

}
