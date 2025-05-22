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

#include "UEBaseApp.h"

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "common/binder/Binder.h"

#include "plexe_5g/packets/TrafficAuthorityPacket_m.h"
#include "plexe_5g/messages/IntersectionClearance_m.h"
#include "plexe_5g/messages/IntersectionClearanceUpdate_m.h"
#include "plexe_5g/messages/IntersectionExit_m.h"
#include "plexe_5g/messages/IntersectionHold_m.h"
#include "plexe_5g/messages/IntersectionRequest_m.h"
#include "plexe_5g/messages/IntersectionUpdate_m.h"
#include "plexe_5g/messages/IntersectionExitNotificationRequest_m.h"
#include "plexe_5g/messages/IntersectionExitNotification_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"
#include "plexe/apps/GeneralPlatooningApp.h"

#include "plexe_5g/UEBaseApp.h"

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_UPDATE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4)
#define PLATOON_SEARCH_SIZE_B (PLATOON_QUERY_SIZE_B + 1)
#define PLATOON_APPROACH_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

#define PLATOONING_TRAFFIC_AUTHORITY_APP_ID 1234

namespace plexe {

using namespace omnetpp;

class UEIntersectionMergeApp: public UEBaseApp
{

    std::string mecAppName;

    //scheduling
    cMessage *selfStart_;

public:
    ~UEIntersectionMergeApp();
    UEIntersectionMergeApp();

protected:

    // general platooning app, storing implementation of maneuvers
    GeneralPlatooningApp* app;

    void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleSelfMsg(cMessage *msg) override;
    virtual void handleMECAppMsg(inet::Packet* msg) override;
    virtual void handleCV2XPacket(inet::Packet* packet) override;
    virtual void handleMECAppStartAck(inet::Packet* packet) override;

    void startManeuver();
    IntersectionRequest* createIntersectionRequest();
    IntersectionUpdate* createIntersectionUpdate();
    IntersectionExit* createIntersectionExit();
    void sendIntersectionRequest();
    void sendIntersectionExitNotification();
    void sendIntersectionExitNotificationRequest();
    void sendIntersectionExit();
    double measureRoadDistanceTo(std::string edge, double pos);
    double evalBrakingPoint();

    void onIntersectionClearance(const IntersectionClearance* msg);
    void onIntersectionClearanceUpdate(const IntersectionClearanceUpdate* msg);
    void onIntersectionHold(const IntersectionHold* msg);
    void onIntersectionExitNotification(const IntersectionExitNotification* msg);
    void onIntersectionExitNotificationRequest(const IntersectionExitNotificationRequest* msg);

private:

    enum class IntersectionManeuverState {
        IDLE,           // maneuver not started
        WAIT_RSU,       // waiting for the RSU to reply (Leader only)
        ENTERING,       // enter the intersection with planned acc / speed (Leader only)
        HOLDING,        // hold & wait at the holding point (Leader only)
        WAIT_EXIT,      // waiting to cross the intersection (Last vehicle only)
        END,            // the last vehicle left the intersection
    };

    IntersectionManeuverState state = IntersectionManeuverState::IDLE;

    std::string commonEdge;
    double distanceThreshold;
    SimTime distanceInterval;
    cMessage* takeDistanceMeasurement = nullptr;
    cMessage* checkIntersectionExit = nullptr;
    cMessage* checkHoldingDistance = nullptr;
    double holdingPosition;
    double holdingAcceleration;

    SimTime rounded;
    cMessage* recordData = nullptr;  // SelfMessage to trigger recording
    cOutVector nodeIdOut;  // Id
    cOutVector speedOut, posxOut, posyOut; // Speed and position
    cOutVector accelerationOut;  // real Acceleration
    cOutVector emissionsOut;   // CO2 emission

    bool usePeriodicUpdates = false;
    double intersectionUpdateInterval = -1;
    cMessage* sendIntersectionUpdate = nullptr;

};

} //namespace
