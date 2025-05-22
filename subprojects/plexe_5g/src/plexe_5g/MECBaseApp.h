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
#include "apps/mec/MecApps/MultiUEMECApp.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

namespace plexe {

using namespace std;
using namespace omnetpp;
using namespace simu5g;

//
//  This is a simple MEC app that receives the coordinates and the radius from the UE app
//  and subscribes to the CircleNotificationSubscription of the Location Service. The latter
//  periodically checks if the UE is inside/outside the area and sends a notification to the
//  MEC App. It then notifies the UE.

//
//  The event behavior flow of the app is:
//  1) receive coordinates from the UE app
//  2) subscribe to the circleNotificationSubscription
//  3) receive the notification
//  4) send the alert event to the UE app
//  5) (optional) receive stop from the UE app
//
// TCP socket management is not fully controlled. It is assumed that connections works
// at the first time (The scenarios used to test the app are simple). If a deeper control
// is needed, feel free to improve it.

//

class MECBaseApp : public MultiUEMECApp
{

    //UDP socket to communicate with the UeApps
    std::vector<struct UE_MEC_CLIENT> ueAddresses;
    inet::UdpSocket ueSocket;
    int localUePort;

    cPar* delay = nullptr;
    bool useDelay = false;

    inet::TcpSocket* serviceSocket_;
    inet::TcpSocket* mp1Socket_;

    HttpBaseMessage* mp1HttpMessage;
    HttpBaseMessage* serviceHttpMessage;

    std::string subId;

    /**
     * Sends a UDP packet to the UE using the given address and port
     * @param packet packet to be sent
     * @param ueAddress IP address of UE
     * @param uePort UDP port of UE application
     */
    void sendInetPacket(inet::UdpSocket s, inet::L3Address address, int port, cPacket* packet);

protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleProcessedMessage(omnetpp::cMessage *msg) override;

    virtual void handleHttpMessage(int connId) override;
    virtual void handleServiceMessage(int connId) override;
    virtual void handleMp1Message(int connId) override;
    virtual void handleUeMessage(omnetpp::cMessage *msg) override;

    virtual void handleSelfMessage(cMessage *msg) override;

    /* TCPSocket::CallbackInterface callback methods */
    virtual void established(int connId) override;

    veins::TraCICommandInterface* traci;

    virtual void sendToUEApp(cPacket* packet, inet::L3Address ueAddress, int uePort);

    // to be overridden by inheriting classes
    virtual void handleUEAppMsg(inet::Packet* packet) {};
    virtual void handleSelfMsg(cMessage *msg) {};

public:
    virtual void addNewUE(struct UE_MEC_CLIENT ueData) override;
    MECBaseApp();
    virtual ~MECBaseApp();

};

}
