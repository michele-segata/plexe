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

#include "MECTrafficAuthorityApp.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"
#include "packets/TrafficAuthorityPacket_m.h"
#include "TrafficAuthorityPacketTypes.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "nodes/mec/utils/httpUtils/httpUtils.h"
#include "nodes/mec/utils/httpUtils/json.hpp"
#include "nodes/mec/MECPlatform/MECServices/packets/HttpResponseMessage/HttpResponseMessage.h"

#include "plexe_5g/PlexeInetUtils.h"
#include "plexe_5g/messages/PlatoonSpeedCommand_m.h"
#include "plexe_5g/messages/PlatoonContactCommand_m.h"

#define LOG EV

namespace plexe {

using namespace inet;
using namespace omnetpp;
using namespace simu5g;

Define_Module(MECTrafficAuthorityApp);

#define PLATOON_QUERY_SIZE_B (4 + 4)
#define PLATOON_SEARCH_RESPONSE_SIZE_B (PLATOON_QUERY_SIZE_B + 4 + 4 + 4 + 1)
#define PLATOON_SPEED_SIZE_B (PLATOON_QUERY_SIZE_B + 4)
#define PLATOON_CONTACT_SIZE_B (PLATOON_QUERY_SIZE_B + 4)

MECTrafficAuthorityApp::MECTrafficAuthorityApp(): MultiUEMECApp()
{
    traci = nullptr;

}
MECTrafficAuthorityApp::~MECTrafficAuthorityApp()
{
}


void MECTrafficAuthorityApp::initialize(int stage)
{
    MecAppBase::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    approachDistanceThreshold = par("approachDistanceThreshold");
    approachSpeedDelta = par("approachSpeedDelta");

    // set Udp Socket
    ueSocket.setOutputGate(gate("socketOut"));

    localUePort = par("localUePort");
    ueSocket.bind(localUePort);

    //testing
    EV << "MECTrafficAuthorityApp::initialize - Mec application "<< getClassName() << " with mecAppId["<< mecAppId << "] has started!" << endl;

    mp1Socket_ = addNewSocket();

    // connect with the service registry
    cMessage *msg = new cMessage("connectMp1");
    scheduleAt(simTime() + 0, msg);

}

void MECTrafficAuthorityApp::finish(){
    MecAppBase::finish();
    EV << "MECTrafficAuthorityApp::finish()" << endl;

    if(gate("socketOut")->isConnected()){

    }
}

void MECTrafficAuthorityApp::addNewUE(struct UE_MEC_CLIENT ueData)
{
    for (auto address : ueAddresses) {
        if (address.address == ueData.address)
            return;
    }
    ueAddresses.push_back(ueData);
}

void MECTrafficAuthorityApp::handleUeMessage(omnetpp::cMessage *msg)
{
    // determine its source address/port
    auto pk = check_and_cast<Packet *>(msg);
    L3Address ueAppAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
    int ueAppPort = pk->getTag<L4PortInd>()->getSrcPort();

    for (size_t i = 0; i < ueAddresses.size(); i++) {
        if (ueAddresses[i].address == ueAppAddress) {
            if (ueAddresses[i].port == -1) {

                // send ACK. check if necessary
                auto ack = inet::makeShared<TrafficAuthorityAppPacket>();
                ack->setType(TA_START_ACK);
                ack->setChunkLength(inet::B(2));
                inet::Packet* packet = new inet::Packet("TrafficAuthorityAppPacket");
                packet->insertAtBack(ack);
                ueSocket.sendTo(packet, ueAppAddress, ueAppPort);

                ueAddresses[i].port = ueAppPort;

            }
            break;
        }
    }

    if (PlatoonUpdateMessage* update = PlexeInetUtils::decapsulate<PlatoonUpdateMessage>(pk)) {
        onPlatoonUpdate(update, ueAppAddress, ueAppPort);
        delete update;
    }
    else if (PlatoonSearchRequest* search = PlexeInetUtils::decapsulate<PlatoonSearchRequest>(pk)) {
        onPlatoonSearch(search);
        delete search;
    }
    else if (PlatoonApproachRequest* request = PlexeInetUtils::decapsulate<PlatoonApproachRequest>(pk)) {
        onPlatoonApproachRequest(request);
        delete request;
    }

    delete msg;

}

void MECTrafficAuthorityApp::onPlatoonUpdate(const PlatoonUpdateMessage* msg, inet::L3Address ueAppAddress, int ueAppPort)
{
    int platoonId = msg->getPlatoonId();
    PlatoonInfo platoonInfo;
    platoonInfo.platoonId = platoonId;
    platoonInfo.platoonLeader = msg->getVehicleId();
    platoonInfo.leaderAddress = ueAppAddress;
    platoonInfo.leaderPort = ueAppPort;
    platoonInfo.x = msg->getX();
    platoonInfo.y = msg->getY();
    platoonInfo.speed = msg->getSpeed();
    platoonData[platoonId] = platoonInfo;
    LOG << "Traffic authority got update from platoon " << platoonId << ": position x=" << platoonInfo.x << " y=" << platoonInfo.y << " speed=" << platoonInfo.speed << std::endl;

    auto approachingManeuver = approachingManeuvers.find(platoonId);
    if (approachingManeuver != approachingManeuvers.end()) {
        // there is an ongoing approaching maneuver for this platoon, so we need to compute the control action
        computeApproachAction(platoonId, approachingManeuver->second);
    }

}

void MECTrafficAuthorityApp::onPlatoonSearch(const PlatoonSearchRequest* msg)
{
    PlatoonInfo searchingPlatoon, matchingPlatoon;
    double minDistance = 1e9, matchingDistance;
    double minSpeedDifference = 1e9, matchingSpeedDifference;
    bool matchingAhead;

    // search data about the platoon currently searching for another one
    auto thisPlatoon = platoonData.find(msg->getPlatoonId());
    // we don't have data about the platoon that sent the query. stop here
    if (thisPlatoon == platoonData.end()) return;
    searchingPlatoon = thisPlatoon->second;

    //    veins::Coord searchingPosition(searchingPlatoon.x, searchingPlatoon.y);
    double searchingSpeed = searchingPlatoon.speed;

    if (!traci) traci = veins::TraCIScenarioManagerAccess().get()->getCommandInterface();

    for (auto platoon = platoonData.begin(); platoon != platoonData.end(); platoon++) {
        // ignore platoon that is currently searching
        if (platoon->first == msg->getPlatoonId()) continue;

        bool ahead;
        double distance = getDistance(searchingPlatoon, platoon->second, ahead);
        double speedDifference = std::abs(searchingSpeed - platoon->second.speed);

        if (msg->getSearchCriterion() == PlatoonSearchCriterion::DISTANCE) {
            if (distance < minDistance) {
                matchingPlatoon = platoon->second;
                matchingDistance = distance;
                matchingSpeedDifference = speedDifference;
                matchingAhead = ahead;
                minDistance = distance;
            }
        }
        else if (msg->getSearchCriterion() == PlatoonSearchCriterion::SPEED) {
            if (speedDifference < minSpeedDifference) {
                matchingPlatoon = platoon->second;
                matchingDistance = distance;
                matchingSpeedDifference = speedDifference;
                matchingAhead = ahead;
                minSpeedDifference = speedDifference;
            }
        }
    }

    sendPlatoonSearchResponse(searchingPlatoon, matchingPlatoon, matchingDistance, matchingSpeedDifference, matchingAhead);

}

void MECTrafficAuthorityApp::sendPlatoonSearchResponse(const PlatoonInfo& searchingPlatoon, const PlatoonInfo& matchingPlatoon, double distance, double speedDifference, bool ahead)
{
    LOG << "Sending response back to platoon number " << searchingPlatoon.platoonId << " with candidate platoon " << matchingPlatoon.platoonId << std::endl;
    PlatoonSearchResponse* msg = new PlatoonSearchResponse("platoonSearchResponse");
    populateResponse(*msg, searchingPlatoon);
    msg->setMatchingPlatoonId(matchingPlatoon.platoonId);
    msg->setDistance(distance);
    msg->setSpeedDifference(speedDifference);
    msg->setAhead(ahead);
    msg->setByteLength(PLATOON_SEARCH_RESPONSE_SIZE_B);
    sendInetPacket(msg, searchingPlatoon.leaderAddress, searchingPlatoon.leaderPort);
}

void MECTrafficAuthorityApp::sendSpeedCommand(const PlatoonInfo& platoon, double speed)
{
    LOG << "Sending speed command to platoon number " << platoon.platoonId << ": set speed to " << speed << "m/s\n";
    PlatoonSpeedCommand* msg = new PlatoonSpeedCommand("platoonSpeedCommand");
    populateResponse(*msg, platoon);
    msg->setSpeed(speed);
    msg->setByteLength(PLATOON_SPEED_SIZE_B);
    sendInetPacket(msg, platoon.leaderAddress, platoon.leaderPort);
}

void MECTrafficAuthorityApp::sendContactPlatoonCommand(const PlatoonInfo& platoon, int contactPlatoonId, int contactLeaderId)
{
    PlatoonContactCommand* msg = new PlatoonContactCommand("platoonContactCommand");
    populateResponse(*msg, platoon);
    msg->setContactPlatoonId(contactPlatoonId);
    msg->setContactLeaderId(contactLeaderId);
    msg->setByteLength(PLATOON_CONTACT_SIZE_B);
    sendInetPacket(msg, platoon.leaderAddress, platoon.leaderPort);
}

void MECTrafficAuthorityApp::populateResponse(PlatoonTAQuery& msg, const PlatoonInfo& destination)
{
    msg.setPlatoonId(destination.platoonId);
    msg.setVehicleId(destination.platoonLeader);
}

void MECTrafficAuthorityApp::onPlatoonApproachRequest(const PlatoonApproachRequest* msg)
{
    computeApproachAction(msg->getPlatoonId(), msg->getApproachId());
}

void MECTrafficAuthorityApp::computeApproachAction(int approachingId, int approachedId)
{
    bool ahead;
    auto approaching = platoonData.find(approachingId);
    auto approached = platoonData.find(approachedId);
    // if one of the two platoons doesn't exist, simply ignore the request
    if (approaching == platoonData.end() || approached == platoonData.end()) return;

    double distance = getDistance(approaching->second, approached->second, ahead);
    LOG << "Computing approach action between " << approaching->second.platoonId << " and " << approached->second.platoonId << ": distance=" << distance << std::endl;
    if (distance < approachDistanceThreshold) {
        LOG << "Distance smaller than threshold. Telling platoon to switch to autonomous mode\n";
        // if the platoon is close enough to the other, they can communicate and coordinate themself
        sendContactPlatoonCommand(approaching->second, approached->second.platoonId, approached->second.platoonLeader);
        // if there is an ongoing maneuver managed by the TA, remove it after giving autonomous control to platoons
        if (auto approachingManeuver = approachingManeuvers.find(approachingId) != approachingManeuvers.end()) {
            approachingManeuvers.erase(approachingManeuver);
        }
    }
    else {
        double approachSpeed;
        // if the platoon to approach is ahead, the approaching one should speed up
        if (ahead) approachSpeed = approached->second.speed + approachSpeedDelta;
        // otherwise it should slow down
        else approachSpeed = approached->second.speed - approachSpeedDelta;
        LOG << "Distance larger than threshold. Approaching platoon is " << (ahead ? "ahead" : "behind") << ". Sending set speed command (" << approachSpeed << "m/s)" << std::endl;
        // send the approach speed to the approaching platoon
        sendSpeedCommand(approaching->second, approachSpeed);
        // keep trace that we are currently sending approach commands to the approaching vehicle
        approachingManeuvers[approaching->second.platoonId] = approached->second.platoonId;
    }
}

double MECTrafficAuthorityApp::getDistance(const PlatoonInfo& first, const PlatoonInfo& second, bool& ahead)
{
    veins::Coord firstPosition(first.x, first.y);
    veins::Coord secondPosition(second.x, second.y);
    // if the vehicles are on the same edge and one is ahead of the other, SUMO cannot compute the route all around the circle
    double distanceFirstToSecond = traci->getDistance(firstPosition, secondPosition, true);
    double distanceSecondToFirst = traci->getDistance(secondPosition, firstPosition, true);
    double distance;
    if (distanceFirstToSecond < distanceSecondToFirst) {
        distance = distanceFirstToSecond;
        ahead = true;
    }
    else {
        distance = distanceSecondToFirst;
        ahead = false;
    }
    return distance;
}

void MECTrafficAuthorityApp::sendInetPacket(cPacket *packet, inet::L3Address ueAddress, int uePort)
{
    inet::Packet *container = PlexeInetUtils::encapsulate(packet, "Plexe_Container");
    ueSocket.sendTo(container, ueAddress, uePort);
}

void MECTrafficAuthorityApp::established(int connId)
{
    if(connId == mp1Socket_->getSocketId())
    {
        EV << "MECTrafficAuthorityApp::established - Mp1Socket"<< endl;
    }
    else if (connId == serviceSocket_->getSocketId())
    {
        EV << "MECTrafficAuthorityApp::established - serviceSocket"<< endl;
        // here we previously sent a START_ACK when established, but this app uses UDP so we will send the START_ACK when we get the first packet
        return;
    }
    else
    {
        throw cRuntimeError("MecAppBase::socketEstablished - Socket %d not recognized", connId);
    }
}


void MECTrafficAuthorityApp::handleHttpMessage(int connId)
{
    if (mp1Socket_ != nullptr && connId == mp1Socket_->getSocketId()) {
        handleMp1Message(connId);
    }
    else
    {
        handleServiceMessage(connId);
    }
}

void MECTrafficAuthorityApp::handleMp1Message(int connId)
{
    // for now I only have just one Service Registry
    HttpMessageStatus *msgStatus = (HttpMessageStatus*) mp1Socket_->getUserData();
    mp1HttpMessage = (HttpBaseMessage*) msgStatus->httpMessageQueue.front();
    EV << "MECTrafficAuthorityApp::handleMp1Message - payload: " << mp1HttpMessage->getBody() << endl;

}

void MECTrafficAuthorityApp::handleServiceMessage(int connId)
{
    HttpMessageStatus *msgStatus = (HttpMessageStatus*) serviceSocket_->getUserData();
    serviceHttpMessage = (HttpBaseMessage*) msgStatus->httpMessageQueue.front();
    EV << "MECTrafficAuthorityApp::handleTcpMsg - REQUEST " << serviceHttpMessage->getBody()<< endl;
    // we do not handle http requests in this app

    if(serviceHttpMessage->getType() == HttpMsgType::REQUEST)
    {
        Http::send204Response(serviceSocket_); // send back 204 no content
    }

}

void MECTrafficAuthorityApp::handleSelfMessage(cMessage *msg)
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
                EV << "MECTrafficAuthorityApp::handleSelfMessage - service IP address is  unspecified (maybe response from the service registry is arriving)" << endl;
            else if(serviceSocket_->getState() == inet::TcpSocket::CONNECTED)
                EV << "MECTrafficAuthorityApp::handleSelfMessage - service socket is already connected" << endl;

            throw cRuntimeError("service socket already connected, or service IP address is unspecified");
        }
    }

    delete msg;
}

void MECTrafficAuthorityApp::handleProcessedMessage(cMessage *msg)
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

