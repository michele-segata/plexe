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

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "nodes/mec/MECPlatform/ServiceRegistry/ServiceRegistry.h"
#include "MECBaseApp.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe_5g/messages/PlatoonUpdateMessage_m.h"
#include "plexe_5g/messages/PlatoonSpeedCommand_m.h"
#include "plexe_5g/messages/PlatoonChangeLaneCommand_m.h"
#include "plexe_5g/messages/StartOvertakeRequest_m.h"

//#include "plexe_5g/Migrator.h"

namespace plexe {

using namespace std;
using namespace omnetpp;
using namespace simu5g;

struct PlatoonInfo {
    int platoonId;
    int platoonLeader;
    // TODO: maybe a socket is not necessary
    inet::UdpSocket* socket;
    inet::L3Address leaderAddress;
    int leaderPort;
    double x;
    double y;
    double speed;
    double time;
};

// id of different platoons
#define OVERTAKING_PLATOON 1
#define OVERTOOK_PLATOON 0
#define INCOMING_PLATOON 2

enum OVERTAKING_STATE {
    WAITING_REQUEST = 0,
    NOT_STARTED,
    OVERTAKING_NOWAIT,
    OVERTAKING_NOWAIT_SLOWDOWN,
    WAITING_PASSBY,
    OVERTAKING_AFTER_PASSBY,
    COMPLETED,
};

enum SAFETY_CONDITION {
    OVERTAKE_NO_SLOWDOWN,
    OVERTAKE_WITH_SLOWDOWN,
    NO_OVERTAKE,
    OVERTAKE_COMPLETED,
};

// strategy chosen within a specific simulation, for logging purposes
enum OVERTAKING_STRATEGY {
    OVERTAKE_WITHOUT_SLOWDOWN = 0,
    OVERTAKE_WITH_INCOMING_SLOWDOWN = 1,
    OVERTAKE_AFTER_PASSBY = 2,
    OVERTAKE_BECAME_UNFEASIBLE = 3,
};

class MECOvertakeApp : public MECBaseApp
{

private:
    // map from platoon id to its information
    typedef std::map<int, struct PlatoonInfo> PlatoonData;

    // information about the overtaking platoon
    PlatoonInfo overtaking;
    // the one being overtook
    PlatoonInfo overtook;
    // and the one coming from the reverse direction
    PlatoonInfo incoming;

    enum OVERTAKING_STATE overtakingState = WAITING_REQUEST;

    // overtaking speed
    double overtakeSpeed;
    // acceleration used to speed up to the overtaking speed
    double overtakeAcceleration = 1.5;
    // initial speed of the incoming traffic to allow restoring it
    double incomingOriginalSpeed;
    // speed factor to slow down incoming traffic to allow overtaking
    double rho;
    // minimum safety margin between overtaking and incoming traffic to avoid head-on collisions
    double epsilon;
    // whether to use or not cooperative overtake, used to get baseline comparison
    bool useCooperativeOvertake;

    enum SAFETY_CONDITION computeOvertakeSafety();
    int overtakeSafetyComputationCounter = 0;

    enum OVERTAKING_STRATEGY usedOvertakingStrategy;

    static const char* OVERTAKING_STATE_TEXT[];
    static const char* SAFETY_CONDITION_TEXT[];

    // set of delays from UEs till APP
    std::map<inet::L3Address, double> delays;

    double computeAverageDelay();

    double v3 = 0;

//    Migrator* migrator = nullptr;

protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;


    virtual void handleUEAppMsg(inet::Packet* packet) override;
    virtual void handleSelfMsg(cMessage *msg) override {};

    veins::TraCICommandInterface* traci;

    /**
     * Handles a platoon update message, which includes information such as speed and position.
     * If there is an ongoing approach maneuver for the platoon sending the update (@see onPlatoonApproachRequest), updates the control command
     */
    virtual void onPlatoonUpdate(const PlatoonUpdateMessage* msg, inet::L3Address ueAppAddress, int ueAppPort);

    virtual void onStartOvertakeRequest(double overtakeSpeed, double overtakeAcceleration);

    /**
     * Sends a command to a platoon to change its speed
     * @param platoon the platoon to which the command should be sent
     * @param speed the speed to be set
     */
    void sendSpeedCommand(const PlatoonInfo& platoon, double speed);

    /**
     * Sends a command to a platoon to change its lane
     * @param platoon the platoon to which the command should be sent
     * @param laneChangeRelative lane change action in terms of relative lanes to change
     */
    void sendChangeLaneCommand(const PlatoonInfo& platoon, int laneChangeRelative);

    /**
     * Utility function used to populate a message with common fields
     */
    void populateResponse(PlatoonTAQuery& msg, const PlatoonInfo& destination);

    /**
     * Computes whether it is feasible to do the overtake or not, and sends the actions to the vehicles.
     */
    void computeOvertakingAction();

    /**
     * Override method so that we transmit only if the migrator says we are allowed to do so
     */
    void sendToUEApp(cPacket* packet, inet::L3Address ueAddress, int uePort) override;

public:
    MECOvertakeApp();
    virtual ~MECOvertakeApp();

};

}
