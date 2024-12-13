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

#include "MECBaseApp.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"
#include "UEMECPacketTypes.h"
#include "UEMECPacket_m.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "nodes/mec/utils/httpUtils/httpUtils.h"
#include "nodes/mec/utils/httpUtils/json.hpp"
#include "nodes/mec/MECPlatform/MECServices/packets/HttpResponseMessage/HttpResponseMessage.h"

#include "plexe_5g/PlexeInetUtils.h"

#define LOG EV

namespace plexe {

using namespace inet;
using namespace omnetpp;
using namespace simu5g;

Define_Module(MECBaseApp);

MECBaseApp::MECBaseApp(): MultiUEMECApp()
{
    traci = nullptr;

}
MECBaseApp::~MECBaseApp()
{
}


void MECBaseApp::initialize(int stage)
{
    MecAppBase::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    // set Udp Socket
    ueSocket.setOutputGate(gate("socketOut"));

    localUePort = par("localUePort");
    ueSocket.bind(localUePort);

    //testing
    EV << "MECBaseApp::initialize - Mec application "<< getClassName() << " with mecAppId["<< mecAppId << "] has started!" << endl;

    mp1Socket_ = addNewSocket();

    // connect with the service registry
    cMessage *msg = new cMessage("connectMp1");
    scheduleAt(simTime() + 0, msg);

}

void MECBaseApp::finish(){
    MecAppBase::finish();
    EV << "MECBaseApp::finish()" << endl;

    if(gate("socketOut")->isConnected()){

    }
}

void MECBaseApp::addNewUE(struct UE_MEC_CLIENT ueData)
{
    for (auto address : ueAddresses) {
        if (address.address == ueData.address)
            return;
    }
    ueAddresses.push_back(ueData);
}

void MECBaseApp::handleUeMessage(omnetpp::cMessage *msg)
{
    // determine its source address/port
    auto pk = check_and_cast<Packet *>(msg);
    L3Address ueAppAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
    int ueAppPort = pk->getTag<L4PortInd>()->getSrcPort();

    for (size_t i = 0; i < ueAddresses.size(); i++) {
        if (ueAddresses[i].address == ueAppAddress) {
            if (ueAddresses[i].port == -1) {

                // send ACK. check if necessary
                auto ack = inet::makeShared<UEMECAppPacket>();
                ack->setType(UEMEC_START_ACK);
                ack->setChunkLength(inet::B(2));
                inet::Packet* packet = new inet::Packet("UEMECAppPacket");
                packet->insertAtBack(ack);
                ueSocket.sendTo(packet, ueAppAddress, ueAppPort);

                ueAddresses[i].port = ueAppPort;

            }
            break;
        }
    }

    handleUEAppMsg(pk);

    delete msg;

}

void MECBaseApp::sendInetPacket(inet::UdpSocket s, inet::L3Address address, int port, cPacket* packet)
{
    std::stringstream ss;
    ss << "Plexe_Container: " << packet->getName();
    inet::Packet *container = PlexeInetUtils::encapsulate(packet, ss.str().c_str());
    s.sendTo(container, address, port);
}

void MECBaseApp::sendToUEApp(cPacket* packet, inet::L3Address ueAddress, int uePort)
{
    sendInetPacket(ueSocket, ueAddress, uePort, packet);
}

void MECBaseApp::established(int connId)
{
    if(connId == mp1Socket_->getSocketId())
    {
        EV << "MECBaseApp::established - Mp1Socket"<< endl;
    }
    else if (connId == serviceSocket_->getSocketId())
    {
        EV << "MECBaseApp::established - serviceSocket"<< endl;
        // here we previously sent a START_ACK when established, but this app uses UDP so we will send the START_ACK when we get the first packet
        return;
    }
    else
    {
        throw cRuntimeError("MecAppBase::socketEstablished - Socket %d not recognized", connId);
    }
}


void MECBaseApp::handleHttpMessage(int connId)
{
    if (mp1Socket_ != nullptr && connId == mp1Socket_->getSocketId()) {
        handleMp1Message(connId);
    }
    else
    {
        handleServiceMessage(connId);
    }
}

void MECBaseApp::handleMp1Message(int connId)
{
    // for now I only have just one Service Registry
    HttpMessageStatus *msgStatus = (HttpMessageStatus*) mp1Socket_->getUserData();
    mp1HttpMessage = (HttpBaseMessage*) msgStatus->httpMessageQueue.front();
    EV << "MECBaseApp::handleMp1Message - payload: " << mp1HttpMessage->getBody() << endl;

}

void MECBaseApp::handleServiceMessage(int connId)
{
    HttpMessageStatus *msgStatus = (HttpMessageStatus*) serviceSocket_->getUserData();
    serviceHttpMessage = (HttpBaseMessage*) msgStatus->httpMessageQueue.front();
    EV << "MECBaseApp::handleTcpMsg - REQUEST " << serviceHttpMessage->getBody()<< endl;
    // we do not handle http requests in this app

    if(serviceHttpMessage->getType() == HttpMsgType::REQUEST)
    {
        Http::send204Response(serviceSocket_); // send back 204 no content
    }

}

void MECBaseApp::handleSelfMessage(cMessage *msg)
{
    if(strcmp(msg->getName(), "connectMp1") == 0)
    {
        EV << "MecAppBase::handleMessage- " << msg->getName() << endl;
        connect(mp1Socket_, mp1Address, mp1Port);
    }

    else if(strcmp(msg->getName(), "connectService") == 0)
    {
        EV << "MecAppBase::handleMessage- " << msg->getName() << endl;
        if(!serviceAddress.isUnspecified() && serviceSocket_->getState() != inet::TcpSocket::CONNECTED)
        {
            connect(serviceSocket_, serviceAddress, servicePort);
        }
        else
        {
            if(serviceAddress.isUnspecified())
                EV << "MECBaseApp::handleSelfMessage - service IP address is  unspecified (maybe response from the service registry is arriving)" << endl;
            else if(serviceSocket_->getState() == inet::TcpSocket::CONNECTED)
                EV << "MECBaseApp::handleSelfMessage - service socket is already connected" << endl;

            throw cRuntimeError("service socket already connected, or service IP address is unspecified");
        }
    }
    else {
        handleSelfMsg(msg);
    }

    delete msg;
}

void MECBaseApp::handleProcessedMessage(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        if(ueSocket.belongsToSocket(msg))
       {
           handleUeMessage(msg);
           return;
       }
    }
    MecAppBase::handleProcessedMessage(msg);
}

} //namespace

