//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "veins/modules/application/platooning/UnicastProtocol.h"

// to get recv power from lower layer
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/modules/phy/DeciderResult80211.h"

using omnetpp::simsignal_t;
using namespace Veins;

Define_Module(UnicastProtocol);

const simsignal_t UnicastProtocol::sigDroppedExceededAttempts = registerSignal("droppedExceededAttempts");
const simsignal_t UnicastProtocol::sigTransmissionAttempts = registerSignal("transmissionAttempts");

void UnicastProtocol::initialize(int stage)
{

    BaseWaveApplLayer::initialize(stage);

    if (stage == 0) {
        // by default we have acks
        enableAck = true;

        // get gates
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        upperControlIn = findGate("upperControlIn");
        upperControlOut = findGate("upperControlOut");

        // get parameters
        queueSize = par("queueSize").longValue();
        maxAttempts = par("maxAttempts").longValue();
        ackTimeout = par("ackTimeout").doubleValue();

        // timeout message
        timeout = new cMessage("timeout");

        // sequence number
        sequenceNumber = 0;

        // at init time, no current messages are waiting for acks obviously
        currentMsg = 0;

        // packet loss rate
        packetLossRate = par("packetLossRate").doubleValue();
    }
}

void UnicastProtocol::handleUpperMsg(cMessage* msg)
{

    UnicastMessage* unicast = dynamic_cast<UnicastMessage*>(msg);
    ASSERT2(unicast, "received a message from app layer which is not of type UnicastMessage");

    // first, set timestamp of intended sending time
    unicast->setTimestamp();

    if (queueSize != 0 && queue.size() == queueSize) {
        // queue is full. cannot enqueue the packet
        UnicastProtocolControlMessage* queueFull = new UnicastProtocolControlMessage();
        queueFull->setControlCommand(FULL_QUEUE);
        send(queueFull, upperControlOut);
        delete msg;
        return;
    }

    // save packet into queue
    queue.push(unicast);
    // if the packet we just inserted is the only one in the queue
    // then tell the protocol to immediately process it
    if (queue.size() == 1) {
        processNextPacket();
    }
}

void UnicastProtocol::handleUpperControl(cMessage* msg)
{

    UnicastProtocolControlMessage* controlMsg = dynamic_cast<UnicastProtocolControlMessage*>(msg);
    ASSERT2(controlMsg, "message coming from control gate is not a control message");

    switch (controlMsg->getControlCommand()) {

    case SET_MAC_ADDRESS:
        macAddress = controlMsg->getCommandValue();
        break;

    case DISABLE_ACKS:
        enableAck = false;
        break;

    case ENABLE_ACKS:
        enableAck = true;
        break;

    default:
        ASSERT2(0, "control message contains unknown command");
        break;
    }

    delete msg;
}

void UnicastProtocol::sendMessageDown(int destination, cPacket* msg, int encapsulatedId, int priority, SimTime timestamp, enum Channels::ChannelNumber channel, short kind)
{

    // this function cannot be called if we are still waiting for the ack
    // for another packet. this unicast protocol is simple, nothing like TCP
    ASSERT2(currentMsg == 0, "trying to send a message while still waiting for the ack of another");

    UnicastMessage* unicast = new UnicastMessage("unicast");

    // set basic fields
    unicast->setSource(macAddress);
    unicast->setDestination(destination);
    // NOTICE: autoincrementing the sequence number
    unicast->setSequenceNumber(sequenceNumber++);
    unicast->setType(DATA);
    // the length of source, destination and sequence number is set to 0 because
    // these are actually fields which are present at the mac layer. this unicast
    // protocol is just implementing the unicast mechanism which is still missing
    // in the 1609.4 MAC implementation. The size of this unicast message will then
    // be the size of the encapsulated message
    unicast->setByteLength(0);
    unicast->setPriority(priority);
    unicast->setTimestamp(timestamp);
    unicast->setKind(kind);
    unicast->setChannel(channel);
    // encapsulate message. NOTICE that we are encapsulating the message directly
    // decapsulated from the message coming from the application. we could use
    // msg->dup(), but then, since msg has been decapsulated, we would have to
    // free it
    unicast->encapsulate(msg);

    WaveShortMessage* wsm = new WaveShortMessage();
    populateWSM(wsm, -1, unicast->getSequenceNumber());
    wsm->setChannelNumber(channel);
    wsm->setUserPriority(priority);
    wsm->encapsulate(unicast);
    // include control info that might have been set at higher layers
    if (msg->getControlInfo()) {
        wsm->setControlInfo(msg->getControlInfo()->dup());
    }
    sendDown(wsm);

    // TODO: check whether to leave this here or somewhere else
    // if we are sending a unicast packet, schedule ack timeout
    if (destination != -1) {
        currentMsg = unicast->dup();
        nAttempts = 1;
        scheduleAt(simTime() + SimTime(ackTimeout), timeout);
    }
    // if we are sending a broadcast, delete the packet from the queue
    else {
        queue.pop();
        processNextPacket();
    }
}

void UnicastProtocol::sendAck(const UnicastMessage* msg)
{

    UnicastMessage* unicast = new UnicastMessage("unicast");

    unicast->setSource(macAddress);
    unicast->setDestination(msg->getSource());
    unicast->setSequenceNumber(msg->getSequenceNumber());
    unicast->setByteLength(0);
    unicast->setPriority(0);
    unicast->setChannel(msg->getChannel());
    unicast->setType(ACK);

    WaveShortMessage* wsm = new WaveShortMessage();
    populateWSM(wsm, -1, msg->getSequenceNumber());
    wsm->setChannelNumber(msg->getChannel());
    wsm->setUserPriority(msg->getPriority());
    wsm->encapsulate(unicast);
    sendDown(wsm);
}

void UnicastProtocol::resendMessage()
{

    WaveShortMessage* wsm = new WaveShortMessage();
    populateWSM(wsm, -1, currentMsg->getSequenceNumber());
    wsm->setChannelNumber(currentMsg->getChannel());
    wsm->setUserPriority(currentMsg->getPriority());
    wsm->encapsulate(currentMsg->dup());
    sendDown(wsm);

    scheduleAt(simTime() + SimTime(ackTimeout), timeout);
    nAttempts++;
}

void UnicastProtocol::handleUnicastMessage(const UnicastMessage* msg)
{

    ASSERT2(msg->getType() == DATA, "handleUnicastMessage cannot handle ACK frames");

    int destination = msg->getDestination();
    int source = msg->getSource();
    //-1 means never seen a message from the sender of the message
    int expectedSequenceNumber = -1;
    std::map<int, int>::iterator sequenceNumberIt;

    if (destination == macAddress) {

        // message is directed to this node

        // first of all check whether this is a duplicate
        sequenceNumberIt = receiveSequenceNumbers.find(source);

        if (sequenceNumberIt != receiveSequenceNumbers.end()) {
            expectedSequenceNumber = sequenceNumberIt->second;
        }

        if (msg->getSequenceNumber() >= expectedSequenceNumber) {
            // we have never seen this message, we have to send it up to to the application
            // notice that we do not decapsulate, because the upper layer may want to know
            // the sender address, so we just pass up the entire frame
            send(msg->dup(), upperLayerOut);

            // update next expected sequence number
            receiveSequenceNumbers[source] = msg->getSequenceNumber() + 1;
        }

        // if it is a new message or a duplicate, we have anyhow to send the ack
        if (enableAck) {
            sendAck(msg);
        }
    }
    else {
        if (destination == -1) {
            // message is broadcast. directed to this node but no need to ack
            send(msg->dup(), upperLayerOut);
            // update next expected sequence number
            receiveSequenceNumbers[source] = msg->getSequenceNumber() + 1;
        }
    }
}

void UnicastProtocol::handleAckMessage(const UnicastMessage* ack)
{

    ASSERT2(ack->getType() == ACK, "handleAckMessage cannot handle DATA frames");

    // if ack is not directed to this node, just drop it
    if (ack->getDestination() != macAddress) {
        return;
    }

    if (currentMsg == 0) {
        // we have received an ack we were not waiting for. do nothing
        EV_DEBUG << "unexpected ACK";
    }
    else {

        int msgDestination, ackSource;
        int msgSequence, ackSequence;

        msgDestination = currentMsg->getDestination();
        ackSource = ack->getSource();
        msgSequence = currentMsg->getSequenceNumber();
        ackSequence = ack->getSequenceNumber();

        // if we received a duplicated ack, we simply ignore it
        if (ackSequence < msgSequence) return;

        ASSERT2(msgDestination == ackSource && msgSequence == ackSequence, "received a wrong ACK");

        emit(sigTransmissionAttempts, nAttempts);

        // we've got the ack. stop timeout timer
        if (timeout->isScheduled()) cancelEvent(timeout);
        // message has been correctly received by the destination. move on to next packet
        queue.pop();
        delete currentMsg;
        currentMsg = 0;
        nAttempts = 0;

        processNextPacket();
    }
}

void UnicastProtocol::handleLowerMsg(cMessage* msg)
{
    // first try to get the WSM out
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT2(wsm, "expecting a WSM but something different received");

    // then get our unicast message out
    UnicastMessage* unicast = dynamic_cast<UnicastMessage*>(wsm->decapsulate());
    ASSERT2(unicast, "no unicast message inside the WSM message");

    // pass up also received power information
    PhyToMacControlInfo* ctlInfo = dynamic_cast<PhyToMacControlInfo*>(wsm->getControlInfo());
    ASSERT2(ctlInfo, "no control info into mac packet");
    DeciderResult80211* res = dynamic_cast<DeciderResult80211*>(ctlInfo->getDeciderResult());
    ASSERT2(res, "no decider result into control info");
    unicast->setRecvPower_dBm(res->getRecvPower_dBm());

    delete wsm;

    double r = dblrand();
    if (r < packetLossRate) {
        // discard the message
        delete unicast;
        return;
    }

    switch (unicast->getType()) {
    case DATA:
        handleUnicastMessage(unicast);
        break;
    case ACK:
        handleAckMessage(unicast);
        break;
    default:
        ASSERT2(0, "unknown unicast message received");
        break;
    }

    delete unicast;
}

void UnicastProtocol::handleSelfMsg(cMessage* msg)
{

    if (msg == timeout) {

        // if we have a timeout, we should have a message to re-send
        ASSERT2(currentMsg != 0, "ack timeout occurred with no current message");

        if (nAttempts < maxAttempts) {
            // we try again to send
            resendMessage();
        }
        else {
            emit(sigDroppedExceededAttempts, true);

            // we tried maxAttempts time with no success. discard the
            // message and tell the error to the application
            UnicastProtocolControlMessage* sendError = new UnicastProtocolControlMessage("sendError");
            sendError->setControlCommand(SEND_FAIL);
            // include the message so that application knows which packet has been dropped
            sendError->encapsulate(currentMsg->dup());
            send(sendError, upperControlOut);

            // the packet that we unsuccessfully tried to send is no more needed. delete it
            queue.pop();
            delete currentMsg;
            currentMsg = 0;
            nAttempts = 0;

            processNextPacket(); // start transmissions again after send fail
        }
    }
}

void UnicastProtocol::processNextPacket()
{

    // the queue is empty. no packet to process
    if (queue.empty()) {
        return;
    }

    // get the message from the queue
    UnicastMessage* toSend = queue.front();

    // send message down
    sendMessageDown(toSend->getDestination(), toSend->decapsulate(), toSend->getEncapsulationId(), toSend->getPriority(), toSend->getTimestamp(), (enum Channels::ChannelNumber) toSend->getChannel(), toSend->getKind());

    delete toSend;
}

void UnicastProtocol::onBeacon(WaveShortMessage* wsm)
{
    ASSERT2(0, "onBeacon invoke when handleLowerMsg() has been overridden");
}
void UnicastProtocol::onData(WaveShortMessage* wsm)
{
    ASSERT2(0, "onData invoke when handleLowerMsg() has been overridden");
}

UnicastProtocol::UnicastProtocol()
{
    timeout = nullptr;
}

UnicastProtocol::~UnicastProtocol()
{
    cancelAndDelete(timeout);
    timeout = nullptr;
}
