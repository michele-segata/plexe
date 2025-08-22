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
#include "plexe_5g/messages/PlatoonSearchRequest_m.h"
#include "plexe_5g/messages/PlatoonSearchResponse_m.h"
#include "plexe_5g/messages/PlatoonApproachRequest_m.h"

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

class MECTrafficAuthorityApp : public MECBaseApp
{

private:
    // map from platoon id to its information
    typedef std::map<int, struct PlatoonInfo> PlatoonData;

    struct PlatoonApproach {
        // id of the platoon approaching the other
        int approachingId;
        // id of the platoon being approached
        int approachedId;
    };

protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;


    //        virtual void modifySubscription();
    //        virtual void sendSubscription();
    //        virtual void sendDeleteSubscription();

    virtual void handleUEAppMsg(inet::Packet* packet) override;
    virtual void handleSelfMsg(cMessage *msg) override {};

    PlatoonData platoonData;
    // map from approaching platoon id to id of platoon being approached
    std::map<int, int> approachingManeuvers;
    // distance from the approached platoon at which the TA stops commanding the approaching platoon
    double approachDistanceThreshold;
    // speed difference to be used to approach a platoon
    double approachSpeedDelta;

    veins::TraCICommandInterface* traci;

    /**
     * Handles a platoon update message, which includes information such as speed and position.
     * If there is an ongoing approach maneuver for the platoon sending the update (@see onPlatoonApproachRequest), updates the control command
     */
    virtual void onPlatoonUpdate(const PlatoonUpdateMessage* msg, inet::L3Address ueAppAddress, int ueAppPort);
    /**
     * Handles a platoon search. A platoon might search for a nearby platoon to merge with.
     * The request includes a simple criterion, i.e., find the platoon that is the closest in terms of distance or of relative speed
     */
    virtual void onPlatoonSearch(const PlatoonSearchRequest* msg);
    /**
     * Handles a request for approaching a platoon.
     * The traffic authority instantiates a new approach maneuver and then periodically (every platoon update) sends speed commands to get the approaching platoon closer to the approached one.
     * This procedure continues until the two platoons are closer than the approachDistanceThreshold parameter.
     * At this point, the control is given to the approaching platoon, which autonomously approaches the other platoon
     */
    virtual void onPlatoonApproachRequest(const PlatoonApproachRequest* msg);

    /**
     * Sends the result of a platoon search back to the requester.
     *
     * @param searchingPlatoon the platoon that sent the request
     * @param matchingPlatoon the platoon matches the criterion of the requester
     * @param distance the distance at which the platoon is located
     * @param speedDifference the relative speed between the searching and the matching platoon
     * @param ahead whether the searching platoon is ahead or behind the matching platoon
     */
    void sendPlatoonSearchResponse(const PlatoonInfo& searchingPlatoon, const PlatoonInfo& matchingPlatoon, double distance, double speedDifference, bool ahead);
    /**
     * Sends a command to a platoon to change its speed
     * @param platoon the platoon to which the command should be sent
     * @param speed the speed to be set
     */
    void sendSpeedCommand(const PlatoonInfo& platoon, double speed);

    /**
     * Tells an approaching platoon to directly contact the approached one and coordinate the merging process autonomously
     */
    void sendContactPlatoonCommand(const PlatoonInfo& platoon, int contactPlatoonId, int contactLeaderId);

    /**
     * Utility function used to populate a message with common fields
     */
    void populateResponse(PlatoonTAQuery& msg, const PlatoonInfo& destination);

    /**
     * Computes what a platoon should do to approach another platoon.
     * If this function finds the platoon to be farther away than the approachDistanceThreshold parameter, it will instruct the approaching platoon to accelerate or slow down to get closed to the approached platoon.
     * If the two platoons are instead closer than the threshold, it stops controlling the approaching platoon and tells it to autonomously merge with the other
     */
    void computeApproachAction(int approachingId, int approachedId);
    /**
     * Computes the driving distance between two platoons
     * @param first first platoon
     * @param second second platoon
     * @param ahead boolean variable that will store whether first is ahead of second or not
     * @return the distance between first and second
     */
    double getDistance(const PlatoonInfo& first, const PlatoonInfo& second, bool& ahead);

public:
    MECTrafficAuthorityApp();
    virtual ~MECTrafficAuthorityApp();

};

}
