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

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "common/binder/Binder.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"
#include "plexe/PlexeManager.h"

namespace plexe {

using namespace omnetpp;

class UEBaseApp: public cSimpleModule
{
private:

    //communication to device app and mec app
    inet::UdpSocket socket;

    int size_;
    simtime_t period_;
    int localPort_;
    int deviceAppPort_;
    inet::L3Address deviceAppAddress_;

    char* sourceSimbolicAddress;            //Ue[x]
    char* deviceSimbolicAppAddress_;              //meHost.virtualisationInfrastructure

    // MEC application endPoint (returned by the device app)
    inet::L3Address mecAppAddress_;
    int mecAppPort_;

    //scheduling
    cMessage *selfMecAppStart_;

    // multicast socket used to send and receive 5G V2X multicast frames
    inet::UdpSocket multicastSocket;
    int multicastDestinationPort;
    inet::L3Address multicastAddress;

    void setMulticastAddress(std::string address);
    void sendInetPacket(inet::UdpSocket s, inet::L3Address address, int port, cPacket* packet);

    /**
     * Sends the first message to the MEC app after receiving the START_ACK.
     * This is just used by the MEC app to find the UDP source port of the UE app
     */
    void sendFirstMessageToMECApp();

    void handleAckStartMECApp(cMessage* msg);

public:
    ~UEBaseApp();
    UEBaseApp();

protected:

    // determines position and role of each vehicle
    BasePositionHelper* positionHelper;
    // traci mobility. used for getting/setting info about the car
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    std::unique_ptr<traci::CommandInterface::Vehicle> plexeTraciVehicle;

    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // to send a packet to the MEC app
    void sendToMECApp(cPacket* packet);
    // to send a packet to other vehicles via 5G C-V2X Mode 1
    void sendCV2XPacket(cPacket* packet);

    // to be overridden by inheriting classes
    virtual void handleSelfMsg(cMessage* msg) {};
    virtual void handleMECAppMsg(inet::Packet* msg) {};
    virtual void handleCV2XPacket(inet::Packet* packet) {};
    virtual void handleMECAppStartAck(inet::Packet* packet) {};

    /**
     * Sends a message requesting the instantiation (or the joining) of a MECApp
     *
     * @param mecAppName: name of the MEC app
     * @param shared: whether the app is exclusive to this UE or shared among other UEs
     * @param devAppId: if the app is shared, specifies a common device app id shared with all UEs
     */
    void sendStartMECApp(std::string mecAppName, bool shared, int devAppId = -1);

};

} //namespace
