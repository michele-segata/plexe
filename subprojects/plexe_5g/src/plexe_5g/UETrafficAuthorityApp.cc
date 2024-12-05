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

#include "UETrafficAuthorityApp.h"
#include "TrafficAuthorityPacketTypes.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "plexe/PlexeManager.h"
#include "plexe_5g/PlexeInetUtils.h"

#include "plexe_5g/messages/PlatoonSearchRequest_m.h"
#include "plexe_5g/messages/PlatoonUpdateMessage_m.h"
#include "plexe_5g/messages/PlatoonApproachRequest_m.h"

namespace plexe {

Define_Module(UETrafficAuthorityApp);

using namespace inet;
using namespace std;
using namespace simu5g;

UETrafficAuthorityApp::UETrafficAuthorityApp()
{
    selfStart_ = NULL;
    sendUpdateMsg = NULL;
}

UETrafficAuthorityApp::~UETrafficAuthorityApp()
{
    cancelAndDelete(selfStart_);
    cancelAndDelete(selfMecAppStart_);
    cancelAndDelete(sendUpdateMsg);

}

void UETrafficAuthorityApp::initialize(int stage)
{
    EV << "UETrafficAuthorityApp::initialize - stage " << stage << endl;
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

    //retrieving car cModule
    ue = this->getParentModule();

    mecAppName = par("mecAppName").stringValue();

    //initializing the auto-scheduling messages
    selfStart_ = new cMessage("selfStart");
    selfMecAppStart_ = new cMessage("selfMecAppStart");

    // get traci interface
    mobility = veins::TraCIMobilityAccess().get(getParentModule());
    ASSERT(mobility);
    traci = mobility->getCommandInterface();
    ASSERT(traci);
    traciVehicle = mobility->getVehicleCommandInterface();
    ASSERT(traciVehicle);
    auto plexe = FindModule<PlexeManager*>::findGlobalModule();
    ASSERT(plexe);
    plexeTraci = plexe->getCommandInterface();
    plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, mobility->getExternalId()));
    positionHelper = FindModule<BasePositionHelper*>::findSubModule(getParentModule());
    ASSERT(positionHelper);
    app = FindModule<GeneralPlatooningApp*>::findSubModule(getParentModule());
    ASSERT(app);

    sendUpdateInterval = par("sendUpdateInterval");
    sendPlatoonSearchTime = par("sendPlatoonSearchTime");

    if (positionHelper->isLeader()) {
        sendUpdateMsg = new cMessage("sendUpdateMsg");
        //starting UETrafficAuthorityApp
        simtime_t startTime = par("startTime");
        EV << "UETrafficAuthorityApp::initialize - starting sendStartMEWarningAlertApp() in " << startTime << " seconds " << endl;
        scheduleAt(simTime() + startTime, selfStart_);

        //testing
        EV << "UETrafficAuthorityApp::initialize - sourceAddress: " << sourceSimbolicAddress << " [" << inet::L3AddressResolver().resolve(sourceSimbolicAddress).str() << "]" << endl;
        EV << "UETrafficAuthorityApp::initialize - destAddress: " << deviceSimbolicAppAddress_ << " [" << deviceAppAddress_.str() << "]" << endl;
        EV << "UETrafficAuthorityApp::initialize - binding to port: local:" << localPort_ << " , dest:" << deviceAppPort_ << endl;
    }

    // dynamically connect the multicast in and out gates to lower stack layers
    cGate* lowerInput = getParentModule()->getSubmodule("at")->getOrCreateFirstUnconnectedGate("in", 0, false, true);
    cGate* lowerOutput = getParentModule()->getSubmodule("at")->getOrCreateFirstUnconnectedGate("out", 0, false, true);
    gate("multicastSocketOut")->connectTo(lowerInput);
    lowerOutput->connectTo(gate("multicastSocketIn"));

    // hardcoded multicast IP and port just to enable broadcast like communication through 5G C-V2X (mode 1)
    multicastDestinationPort = 6789;
    multicastSocket.setOutputGate(gate("multicastSocketOut"));
    multicastSocket.bind(multicastDestinationPort);
    setMulticastAddress(std::string("224.0.0.1"));
}

void UETrafficAuthorityApp::setMulticastAddress(std::string address)
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

void UETrafficAuthorityApp::handleMessage(cMessage *msg)
{
    EV << "UETrafficAuthorityApp::handleMessage" << endl;
    // Sender Side
    if (msg->isSelfMessage()) {

        if (msg == sendUpdateMsg) {
            sendUpdateToTA();
            return;
        }

        if (msg == sendPlatoonSearchMsg) {
            sendPlatoonSearch();
            delete sendPlatoonSearchMsg;
            sendPlatoonSearchMsg = nullptr;
            return;
        }

        if (!strcmp(msg->getName(), "selfStart"))
            sendStartMETrafficAuthorityApp();

        else if (!strcmp(msg->getName(), "selfMecAppStart")) {
            sendMessageToMECApp();
            scheduleAt(simTime() + period_, selfMecAppStart_);
        }

        else
            throw cRuntimeError("UETrafficAuthorityApp::handleMessage - \tWARNING: Unrecognized self message");
    }
    else if (strcmp(msg->getArrivalGate()->getName(), "multicastSocketIn") == 0) {

        handleMulticastPacket(msg);

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
                throw cRuntimeError("UETrafficAuthorityApp::handleMessage - \tFATAL! Error when casting to DeviceAppPacket");

            if (!strcmp(mePkt->getType(), ACK_START_MECAPP))
                handleAckStartMETrafficAuthorityApp(msg);

            else {
                throw cRuntimeError("UETrafficAuthorityApp::handleMessage - \tFATAL! Error, DeviceAppPacket type %s not recognized", mePkt->getType());
            }
        }
        // From MEC application
        else {
            if (PlatoonSearchResponse* response = PlexeInetUtils::decapsulate<PlatoonSearchResponse>(packet)) {
                onPlatoonSearchResponse(response);
                delete response;
            }
            else if (PlatoonSpeedCommand* speedCommand = PlexeInetUtils::decapsulate<PlatoonSpeedCommand>(packet)) {
                onPlatoonSpeedCommand(speedCommand);
                delete speedCommand;
            }
            else if (PlatoonContactCommand* contactCommand = PlexeInetUtils::decapsulate<PlatoonContactCommand>(packet)) {
                onPlatoonContactCommand(contactCommand);
                delete contactCommand;
            }
            else {
                auto mePkt = packet->peekAtFront<TrafficAuthorityAppPacket>();
                if (mePkt == 0)
                    throw cRuntimeError("UETrafficAuthorityApp::handleMessage - \tFATAL! Error when casting to TrafficAuthorityAppPacket");

                if (!strcmp(mePkt->getType(), TA_MESSAGE))
                    handleInfoMETrafficAuthorityApp(msg);
                else if (!strcmp(mePkt->getType(), TA_START_NACK)) {
                    EV << "UETrafficAuthorityApp::handleMessage - MEC app did not started correctly, trying to start again" << endl;
                }
                else if (!strcmp(mePkt->getType(), TA_START_ACK)) {
                    EV << "UETrafficAuthorityApp::handleMessage - MEC app started correctly" << endl;
                    if (selfMecAppStart_->isScheduled()) {
                        cancelEvent(selfMecAppStart_);
                    }
                }
                else {
                    throw cRuntimeError("UETrafficAuthorityApp::handleMessage - \tFATAL! Error, TrafficAuthorityAppPacket type %s not recognized", mePkt->getType());
                }
            }
        }
        delete msg;
    }
}

void UETrafficAuthorityApp::sendUpdateToTA()
{
    PlatoonUpdateMessage *msg = new PlatoonUpdateMessage();
    populatePlatooningTAQuery(msg);
    veins::Coord pos = mobility->getPositionAt(simTime());
    msg->setX(pos.x);
    msg->setY(pos.y);
    msg->setSpeed(mobility->getSpeed());
    msg->setByteLength(PLATOON_UPDATE_SIZE_B);
    sendInetPacket(msg);
    scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " sending update to traffic authority\n";
}

void UETrafficAuthorityApp::sendPlatoonSearch()
{
    PlatoonSearchRequest *msg = new PlatoonSearchRequest("PlatoonSearch");
    populatePlatooningTAQuery(msg);
    PlatoonSearchCriterion criterion = PlatoonSearchCriterion::DISTANCE;
    msg->setSearchCriterion(criterion);
    msg->setByteLength(PLATOON_SEARCH_SIZE_B);
    sendInetPacket(msg);
    LOG << "Platoon " << positionHelper->getPlatoonId() << " searching for nearby platoons\n";
}

void UETrafficAuthorityApp::sendPlatoonApproachRequest(int platoonId)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " requesting to approach platoon " << platoonId << "\n";
    PlatoonApproachRequest *msg = new PlatoonApproachRequest();
    populatePlatooningTAQuery(msg);
    msg->setApproachId(platoonId);
    msg->setByteLength(PLATOON_APPROACH_SIZE_B);
    sendInetPacket(msg);
    mySpeed = mobility->getSpeed();
}

void UETrafficAuthorityApp::populatePlatooningTAQuery(PlatoonTAQuery *msg)
{
    msg->setPlatoonId(positionHelper->getPlatoonId());
    msg->setVehicleId(positionHelper->getId());
    msg->setQueryType(REQUEST);
}

void UETrafficAuthorityApp::onPlatoonSearchResponse(PlatoonSearchResponse *msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " got answer from TA: matching platoon ID = " << msg->getMatchingPlatoonId() << "\n";
    sendPlatoonApproachRequest(msg->getMatchingPlatoonId());
}

void UETrafficAuthorityApp::onPlatoonSpeedCommand(PlatoonSpeedCommand *msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " speed command from TA: setting speed to " << msg->getSpeed() << "m/s\n";
    plexeTraciVehicle->setCruiseControlDesiredSpeed(msg->getSpeed());
}

void UETrafficAuthorityApp::onPlatoonContactCommand(PlatoonContactCommand *msg)
{
    LOG << "Platoon " << positionHelper->getPlatoonId() << " contact command fromt TA: contacting platoon " << msg->getContactPlatoonId() << " (leader: " << msg->getContactLeaderId() << ")\n";
    plexeTraciVehicle->setCruiseControlDesiredSpeed(mySpeed);
    app->startMergeManeuver(msg->getContactPlatoonId(), msg->getContactLeaderId(), -1);
}

void UETrafficAuthorityApp::sendInetPacket(cPacket *packet)
{
    inet::Packet *container = PlexeInetUtils::encapsulate(packet, "Plexe_Container");
    socket.sendTo(container, mecAppAddress_, mecAppPort_);
}

void UETrafficAuthorityApp::sendBroadcast(cPacket *pkt)
{
    inet::Packet* container = PlexeInetUtils::encapsulate(pkt, "Plexe_Container");
    multicastSocket.sendTo(container, multicastAddress, multicastDestinationPort);
}

void UETrafficAuthorityApp::handleMulticastPacket(cMessage* msg)
{
    inet::Packet* container = check_and_cast<inet::Packet*>(msg);
    if (container) {
        // TODO: decapsulate as needed
        // PlatoonUpdateMessage* frame = PlexeInetUtils::decapsulate<PlatoonUpdateMessage>(container);
    }
    delete container;
}

void UETrafficAuthorityApp::finish()
{

}
/*
 * -----------------------------------------------Sender Side------------------------------------------
 */
void UETrafficAuthorityApp::sendStartMETrafficAuthorityApp()
{
    inet::Packet *packet = new inet::Packet("TrafficAuthorityPacketStart");
    auto start = inet::makeShared<DeviceAppStartPacket>();

    //instantiation requirements and info
    start->setType(START_MECAPP);
    start->setMecAppName(mecAppName.c_str());
    // tell the device app that this app should be shared among multiple UEs
    start->setShared(true);
    start->setAssociateDevAppId(PLATOONING_TRAFFIC_AUTHORITY_APP_ID);

    start->setChunkLength(inet::B(2 + mecAppName.size() + 1));
    start->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());

    packet->insertAtBack(start);

    socket.sendTo(packet, deviceAppAddress_, deviceAppPort_);

    //rescheduling
    scheduleAt(simTime() + period_, selfStart_);
}

/*
 * ---------------------------------------------Receiver Side------------------------------------------
 */
void UETrafficAuthorityApp::handleAckStartMETrafficAuthorityApp(cMessage *msg)
{
    inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<DeviceAppStartAckPacket>();

    if (pkt->getResult() == true) {
        mecAppAddress_ = L3AddressResolver().resolve(pkt->getIpAddress());
        mecAppPort_ = pkt->getPort();
        EV << "UETrafficAuthorityApp::handleAckStartMETrafficAuthorityApp - Received " << pkt->getType() << " type TrafficAuthorityPacket. mecApp isntance is at: " << mecAppAddress_ << ":" << mecAppPort_ << endl;
        cancelEvent(selfStart_);
        sendMessageToMECApp();
        // TODO: what is this selfMecAppStart_???
        scheduleAt(simTime() + period_, selfMecAppStart_);
        scheduleAt(simTime() + sendUpdateInterval, sendUpdateMsg);

        if (positionHelper->getPlatoonId() == 1) {
            sendPlatoonSearchMsg = new cMessage("sendPlatoonSearchMsg");
            scheduleAt(simTime() + sendPlatoonSearchTime, sendPlatoonSearchMsg);
        }
    }
    else {
        EV << "UETrafficAuthorityApp::handleAckStartMETrafficAuthorityApp - MEC application cannot be instantiated! Reason: " << pkt->getReason() << endl;
    }
}

void UETrafficAuthorityApp::sendMessageToMECApp()
{

    // send star monitoring message to the MEC application

    inet::Packet *pkt = new inet::Packet("TrafficAuthorityPacketStart");
    auto alert = inet::makeShared<TrafficAuthorityPacket>();
    alert->setType(TA_START_TA);
    alert->setChunkLength(inet::B(20));
    alert->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    pkt->insertAtBack(alert);

    socket.sendTo(pkt, mecAppAddress_, mecAppPort_);
    EV << "UETrafficAuthorityApp::sendMessageToMECApp() - start Message sent to the MEC app" << endl;
}

void UETrafficAuthorityApp::handleInfoMETrafficAuthorityApp(cMessage *msg)
{
    inet::Packet *packet = check_and_cast<inet::Packet*>(msg);
    if (!packet) {
        delete msg;
        return;
    }
    if (PlatoonSearchResponse *response = PlexeInetUtils::decapsulate<PlatoonSearchResponse>(packet)) {
        onPlatoonSearchResponse(response);
        delete response;
    }
    else if (PlatoonSpeedCommand *speedCommand = PlexeInetUtils::decapsulate<PlatoonSpeedCommand>(packet)) {
        onPlatoonSpeedCommand(speedCommand);
        delete speedCommand;
    }
    else if (PlatoonContactCommand *contactCommand = PlexeInetUtils::decapsulate<PlatoonContactCommand>(packet)) {
        onPlatoonContactCommand(contactCommand);
        delete contactCommand;
    }
    delete msg;
}

} //namespace

