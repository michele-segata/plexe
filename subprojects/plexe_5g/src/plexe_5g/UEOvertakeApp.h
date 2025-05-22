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

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"
#include "plexe/apps/GeneralPlatooningApp.h"

#include "plexe_5g/messages/PlatoonTAQuery_m.h"
#include "plexe_5g/messages/PlatoonUpdateMessage_m.h"
#include "plexe_5g/messages/PlatoonSpeedCommand_m.h"
#include "plexe_5g/messages/PlatoonChangeLaneCommand_m.h"
#include "plexe_5g/messages/StartOvertakeRequest_m.h"

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_UPDATE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4)
#define PLATOON_SEARCH_SIZE_B (PLATOON_QUERY_SIZE_B + 1)
#define PLATOON_APPROACH_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

#define PLATOONING_OVERTAKE_APP_ID 1245

namespace plexe {

using namespace omnetpp;

class UEOvertakeApp: public UEBaseApp
{

    std::string mecAppName;

    //scheduling
    cMessage *selfStart_;

    double overtakeSpeed = 0;
    double overtakeAcceleration = 0;

public:
    ~UEOvertakeApp();
    UEOvertakeApp();

protected:

    // self message for sending updates to the traffic authority
    cMessage* sendUpdateMsg = nullptr;
    // self message for sending a platoon search query
    cMessage* sendStartOvertakeMsg = nullptr;

    double sendUpdateInterval;
    double sendStartOvertakeTime;

    // general platooning app, storing implementation of maneuvers
    GeneralPlatooningApp* app;

    void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleSelfMsg(cMessage *msg) override;
    virtual void handleMECAppMsg(inet::Packet* msg) override;
    virtual void handleCV2XPacket(inet::Packet* packet) override;
    virtual void handleMECAppStartAck(inet::Packet* packet) override;

    // sends a platoon update (platoon id, position) to the MEC app authority
    void sendUpdateToMECApp();
    // sends a message to start the overtake
    void sendStartOvertakeMessage();
    // helper method to populate traffic authority queries packets
    void populatePlatooningTAQuery(PlatoonTAQuery* msg);
    // invoked when a change speed command is received
    void onPlatoonSpeedCommand(PlatoonSpeedCommand* msg);
    // invoked when a command to contact a platoon is received from the traffic authority
    void onPlatoonChangeLaneCommand(PlatoonChangeLaneCommand* msg);

private:

    cMessage* recordData = nullptr;
    // id
    cOutVector nodeIdOut;
    // speed and position
    cOutVector speedOut, posxOut, posyOut;
    // real acceleration
    cOutVector accelerationOut;
    // co2 emissions
    cOutVector emissionsOut;

    int deviceAppId;

};

} //namespace
