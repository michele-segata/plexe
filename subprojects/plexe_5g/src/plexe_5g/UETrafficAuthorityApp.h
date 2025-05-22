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

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"
#include "plexe/apps/GeneralPlatooningApp.h"

#include "plexe_5g/messages/PlatoonTAQuery_m.h"
#include "plexe_5g/messages/PlatoonSearchResponse_m.h"
#include "plexe_5g/messages/PlatoonSpeedCommand_m.h"
#include "plexe_5g/messages/PlatoonContactCommand_m.h"

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_UPDATE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4)
#define PLATOON_SEARCH_SIZE_B (PLATOON_QUERY_SIZE_B + 1)
#define PLATOON_APPROACH_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

#define PLATOONING_TRAFFIC_AUTHORITY_APP_ID 1234

namespace plexe {

using namespace omnetpp;

/**
 * This is a UE app that asks to a Device App to instantiate the MECWarningAlertApp.
 * After a successful response, it asks to the MEC app to be notified when the car
 * enters a circle zone described by x,y center position and the radius. When a danger
 * event arrives, the car colors becomes red.
 *
 * The event behavior flow of the app is:
 * 1) send create MEC app to the Device App
 * 2.1) ACK --> send coordinates to MEC app
 * 2.2) NACK --> do nothing
 * 3) wait for danger events
 * 4) send delete MEC app to the Device App
 */

class UETrafficAuthorityApp: public UEBaseApp
{

    std::string mecAppName;

    //scheduling
    cMessage *selfStart_;

public:
    ~UETrafficAuthorityApp();
    UETrafficAuthorityApp();

protected:

    // indicates whether we are connected to the traffic authority
    bool connectedToTA;
    // indicates how frequently update messages should be sent to the traffic authority
    simtime_t sendUpdateInterval;
    // indicates when the platoon should send a message for searching for other platoons
    simtime_t sendPlatoonSearchTime;

    // self message for sending updates to the traffic authority
    cMessage* sendUpdateMsg;
    // self message for sending a platoon search query
    cMessage* sendPlatoonSearchMsg;
    // original speed of this platoon
    double mySpeed;

    // general platooning app, storing implementation of maneuvers
    GeneralPlatooningApp* app;

    void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleSelfMsg(cMessage *msg) override;
    virtual void handleMECAppMsg(inet::Packet* msg) override;
    virtual void handleCV2XPacket(inet::Packet* packet) override;
    virtual void handleMECAppStartAck(inet::Packet* packet) override;
//    virtual void handleHandover(bool b) override;

    // sends a platoon update (platoon id, position) to the traffic authority
    void sendUpdateToTA();
    // sends a query to the traffic authority searching for other platoons
    void sendPlatoonSearch();
    // sends a query to the traffic authority asking help for approaching another platoon
    void sendPlatoonApproachRequest(int platoonId);
    // helper method to populate traffic authority queries packets
    void populatePlatooningTAQuery(PlatoonTAQuery* msg);
    // invoked when a platooning search response have been received
    void onPlatoonSearchResponse(PlatoonSearchResponse* msg);
    // invoked when a change speed command is received
    void onPlatoonSpeedCommand(PlatoonSpeedCommand* msg);
    // invoked when a command to contact a platoon is received from the traffic authority
    void onPlatoonContactCommand(PlatoonContactCommand* msg);

};

} //namespace
