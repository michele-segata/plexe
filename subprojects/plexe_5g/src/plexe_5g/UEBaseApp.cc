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

#include "UEBaseApp.h"

#include "UEMECPacketTypes.h"
#include "UEMECPacket_m.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "plexe_5g/PlexeInetUtils.h"

#include "veins/base/utils/FindModule.h"

namespace plexe {

Define_Module(UEBaseApp);

using namespace inet;
using namespace std;
using namespace simu5g;

UEBaseApp::UEBaseApp()
{

}

UEBaseApp::~UEBaseApp()
{
    cancelAndDelete(selfMecAppStart_);
}

void UEBaseApp::initialize(int stage)
{
    EV << "UEBaseApp::initialize - stage " << stage << endl;
    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    //retrieve parameters
    size_ = par("packetSize");
    period_ = par("period");
    localPort_ = par("localPort");
    deviceAppPort_ = par("deviceAppPort");
    sourceSimbolicAddress = (char*) getParentModule()->getFullName();
    deviceSimbolicAppAddress_ = (char*) par("deviceAppAddress").stringValue();
    deviceAppAddress_ = inet::L3AddressResolver().resolve(deviceSimbolicAppAddress_);

    //binding socket
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort_);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

    //initializing the auto-scheduling messages
    selfMecAppStart_ = new cMessage("selfMecAppStart");

    // get traci interface
    mobility = veins::TraCIMobilityAccess().get(getParentModule());
    ASSERT(mobility);
    traci = mobility->getCommandInterface();
    ASSERT(traci);
    traciVehicle = mobility->getVehicleCommandInterface();
    ASSERT(traciVehicle);
    auto plexe = veins::FindModule<PlexeManager*>::findGlobalModule();
    ASSERT(plexe);
    plexeTraci = plexe->getCommandInterface();
    plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, mobility->getExternalId()));
    positionHelper = veins::FindModule<BasePositionHelper*>::findSubModule(getParentModule());
    ASSERT(positionHelper);

    EV << "UEBaseApp::initialize - sourceAddress: " << sourceSimbolicAddress << " [" << inet::L3AddressResolver().resolve(sourceSimbolicAddress).str() << "]" << endl;
    EV << "UEBaseApp::initialize - destAddress: " << deviceSimbolicAppAddress_ << " [" << deviceAppAddress_.str() << "]" << endl;
    EV << "UEBaseApp::initialize - binding to port: local:" << localPort_ << " , dest:" << deviceAppPort_ << endl;

    // dynamically connect the multicast in and out gates to lower stack layers
    cGate* lowerInput = getParentModule()->getSubmodule("at")->getOrCreateFirstUnconnectedGate("in", 0, false, true);
    cGate* lowerOutput = getParentModule()->getSubmodule("at")->getOrCreateFirstUnconnectedGate("out", 0, false, true);
    gate("multicastSocketOut")->connectTo(lowerInput);
    lowerOutput->connectTo(gate("multicastSocketIn"));

    // multicast IP and port to enable broadcast like communication through 5G C-V2X (mode 1)
    multicastDestinationPort = par("multicastPort");
    multicastSocket.setOutputGate(gate("multicastSocketOut"));
    multicastSocket.bind(multicastDestinationPort);
    setMulticastAddress(par("multicastAddress"));
}

void UEBaseApp::setMulticastAddress(std::string address)
{
    if (!multicastAddress.isUnspecified()) {
        // we are already bound to a multicast address. leave this group first
        multicastSocket.leaveMulticastGroup(multicastAddress);
    }
    multicastAddress = inet::L3AddressResolver().resolve(address.c_str());
    inet::IInterfaceTable* ift = inet::getModuleFromPar<inet::IInterfaceTable>(par("interfaceTableModule"), this);
    inet::NetworkInterface* ie = ift->findInterfaceByName("cellular");
    if (!ie)
        throw cRuntimeError("Wrong multicastInterface setting: no interface named cellular");
    multicastSocket.setMulticastOutputInterface(ie->getInterfaceId());
    multicastSocket.joinMulticastGroup(multicastAddress, ie->getInterfaceId());
}

void UEBaseApp::handleMessage(cMessage *msg)
{
    EV << "UEBaseApp::handleMessage" << endl;
    // Sender Side
    if (msg->isSelfMessage()) {

        if (msg == selfMecAppStart_) {
            sendFirstMessageToMECApp();
            scheduleAt(simTime() + period_, selfMecAppStart_);
        }
        else
            handleSelfMsg(msg);
    }
    else if (strcmp(msg->getArrivalGate()->getName(), "multicastSocketIn") == 0) {

        inet::Packet* container = check_and_cast<inet::Packet*>(msg);
        if (container) {
            // TODO: decapsulate as needed within method
            // e.g.: MyMessageType* frame = PlexeInetUtils::decapsulate<MyMessageType>(container);
            handleCV2XPacket(container);
        }
        delete container;

    }
    // Receiver Side
    else {
        inet::Packet *packet = check_and_cast<inet::Packet*>(msg);

        inet::L3Address ipAdd = packet->getTag<L3AddressInd>()->getSrcAddress();
        // int port = packet->getTag<L4PortInd>()->getSrcPort();

        /*
         * From Device app
         * device app usually runs in the UE (loopback), but it could also run in other places
         */
        if (ipAdd == deviceAppAddress_ || ipAdd == inet::L3Address("127.0.0.1")) // dev app
        {
            auto mePkt = packet->peekAtFront<DeviceAppPacket>();

            if (mePkt == 0)
                throw cRuntimeError("UEBaseApp::handleMessage - \tFATAL! Error when casting to DeviceAppPacket");

            if (!strcmp(mePkt->getType(), ACK_START_MECAPP))
                handleAckStartMECApp(msg);

            else {
                throw cRuntimeError("UEBaseApp::handleMessage - \tFATAL! Error, DeviceAppPacket type %s not recognized", mePkt->getType());
            }
        }
        // From MEC application
        else {
            if (!strcmp(packet->getName(), "UEMECAppPacket")) {
                auto mePkt = packet->peekAtFront<UEMECAppPacket>();
                if (mePkt != 0) {
                    if (!strcmp(mePkt->getType(), UEMEC_START_NACK)) {
                        EV << "UEBaseApp::handleMessage - MEC app did not started correctly, trying to start again" << endl;
                    }
                    else if (!strcmp(mePkt->getType(), UEMEC_START_ACK)) {
                        EV << "UEBaseApp::handleMessage - MEC app started correctly" << endl;
                        if (selfMecAppStart_->isScheduled()) {
                            cancelEvent(selfMecAppStart_);
                        }
                    }
                    else {
                        throw cRuntimeError("UEBaseAppApp::handleMessage - \tFATAL! Error, UEMECAppPacket type %s not recognized", mePkt->getType());
                    }
                }
                else {
                    throw cRuntimeError("UEBaseAppApp::handleMessage - \tFATAL! Error, UEMECAppPacket type %s not recognized", mePkt->getType());
                }
            }
            else {
                inet::Packet* container = check_and_cast<inet::Packet*>(msg);
                if (container) {
                    // TODO: decapsulate as needed within method
                    // e.g.: MyMessageType* frame = PlexeInetUtils::decapsulate<MyMessageType>(container);
                    handleMECAppMsg(container);
                }
            }
        }
        delete msg;
    }
}

void UEBaseApp::sendInetPacket(inet::UdpSocket s, inet::L3Address address, int port, cPacket* packet)
{
    std::stringstream ss;
    ss << "Plexe_Container: " << packet->getName();
    inet::Packet *container = PlexeInetUtils::encapsulate(packet, ss.str().c_str());
    s.sendTo(container, address, port);
}

void UEBaseApp::sendToMECApp(cPacket *packet)
{
    sendInetPacket(socket, mecAppAddress_, mecAppPort_, packet);
}

void UEBaseApp::sendCV2XPacket(cPacket *packet)
{
    sendInetPacket(multicastSocket, multicastAddress, multicastDestinationPort, packet);
}

void UEBaseApp::finish()
{

}

void UEBaseApp::sendStartMECApp(std::string mecAppName, bool shared, int devAppId)
{
    inet::Packet *packet = new inet::Packet(START_MECAPP);
    auto start = inet::makeShared<DeviceAppStartPacket>();

    //instantiation requirements and info
    start->setType(START_MECAPP);
    start->setMecAppName(mecAppName.c_str());
    // tell the device app that this app should be shared among multiple UEs
    start->setShared(shared);
    start->setAssociateDevAppId(devAppId);

    start->setChunkLength(inet::B(2 + mecAppName.size() + 1));
    start->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());

    packet->insertAtBack(start);

    socket.sendTo(packet, deviceAppAddress_, deviceAppPort_);
}

void UEBaseApp::handleAckStartMECApp(cMessage *msg)
{
    inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<DeviceAppStartAckPacket>();

    if (pkt->getResult() == true) {
        mecAppAddress_ = L3AddressResolver().resolve(pkt->getIpAddress());
        mecAppPort_ = pkt->getPort();
        EV << "UEBaseApp::handleAckStartMECApp - Received " << pkt->getType() << " typeUEMECAppPacket. mecApp instance is at: " << mecAppAddress_ << ":" << mecAppPort_ << endl;
        // send the first message just to let the MEC app know about the source UDP port of the UE app
        sendFirstMessageToMECApp();
        // reschedule this in case it gets lost and we don't get the ack
        scheduleAt(simTime() + period_, selfMecAppStart_);
        handleMECAppStartAck(packet);
    }
    else {
        EV << "UEBaseApp::handleAckStartMECApp - MEC application cannot be instantiated! Reason: " << pkt->getReason() << endl;
    }
}

void UEBaseApp::sendFirstMessageToMECApp()
{
    inet::Packet *pkt = new inet::Packet(UEMEC_START);
    auto alert = inet::makeShared<UEMECPacket>();
    alert->setType(UEMEC_START);
    alert->setChunkLength(inet::B(20));
    alert->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    pkt->insertAtBack(alert);
    socket.sendTo(pkt, mecAppAddress_, mecAppPort_);
    EV << "UEBaseApp::sendFirstMessageToMECApp() - start Message sent to the MEC app" << endl;
}

} //namespace

