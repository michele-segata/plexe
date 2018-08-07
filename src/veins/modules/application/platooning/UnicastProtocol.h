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

#ifndef UNICASTPROTOCOL_H_
#define UNICASTPROTOCOL_H_

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

#include <queue>

#include "veins/modules/application/platooning/messages/UnicastMessage_m.h"
#include "veins/modules/application/platooning/messages/UnicastProtocolControlMessage_m.h"

enum ControlMessageCommand {
    // from app to protocol for setting mac address
    SET_MAC_ADDRESS,
    // from app to protocol, disable acks reply for unicast (debug purpose)
    DISABLE_ACKS,
    // from app to protocol, enable acks reply
    ENABLE_ACKS,
    // from protocol to app to tell that sending failed after trying several attempts
    SEND_FAIL,
    // from protocol to app to tell that message cannot be sent because queue is full
    FULL_QUEUE
};

class UnicastProtocol : public Veins::BaseWaveApplLayer {

protected:
    // this is the mac address of the node.
    // the application can set it at the beginning
    // so that it does not have to care about it
    // anymore. the unicast layer will insert the
    // mac address before sending a frame and
    // will drop frames not directed to this
    // vehicle
    int macAddress;

    // queue of packets from above
    std::queue<UnicastMessage*> queue;
    // size of the queue, 0 means infinite
    size_t queueSize;
    // maximum number of retransmission attempts when ack not received
    int maxAttempts;
    // amount of time to wait before declaring ack timeout

    // a copy of the current message sent on the channel, waiting for ack
    UnicastMessage* currentMsg;
    // number of retransmission attempts for the current msg
    int nAttempts;

    // ack timeout time
    double ackTimeout;
    // self scheduled message for ack waiting timeout
    cMessage* timeout;
    // enable/disable ack reply on unicast (for debug purpose)
    bool enableAck;

    // sequence number used in the next message
    int sequenceNumber;
    // variable to map a node's MAC address to the next sequence number
    // expected from that node. this is needed for understanding whether
    // the received message is a duplicate or not. notice that the
    // sequence number does not work like in TCP. each node uses a sequence
    // number for all the receivers, and not one for each receiver. So
    // if node A is sending packet 1 to B, and then it wants to send a packet
    // to C, A will use sequence number 2. B will anyhow save the tuple (A,2)
    // saying that the next expected sequence number from A is 2. If A now
    // sends a packet to B, it will use the sequence number 3, but B will
    // anyhow understand that packet 3 is a new packet from A.
    std::map<int, int> receiveSequenceNumbers;

    // packet loss rate (between 0 and 1)
    double packetLossRate;

    // input/output gates from/to upper layer
    int upperLayerIn, upperLayerOut, upperControlIn, upperControlOut;

public:
    static const omnetpp::simsignal_t sigDroppedExceededAttempts;
    static const omnetpp::simsignal_t sigTransmissionAttempts;

    virtual void initialize(int stage);

protected:
    virtual void onBeacon(Veins::WaveShortMessage* wsm);
    virtual void onData(Veins::WaveShortMessage* wsm);

protected:
    virtual void handleUpperMsg(cMessage* msg);
    virtual void handleUpperControl(cMessage* msg);
    virtual void handleSelfMsg(cMessage* msg);
    virtual void handleLowerMsg(cMessage* msg);

protected:
    /**
     * Handle unicast procedures, e.g., determining whether the
     * message is directed to this node, sending the ack, etc.
     */
    virtual void handleUnicastMessage(const UnicastMessage* msg);

    /**
     * Handle the reception of an ack frame
     */
    virtual void handleAckMessage(const UnicastMessage* ack);

    /**
     * Generates and sends a message. if a unicast address is specified, then the message is
     * repeatedly sent until an ack is received. if no ack is received after a certain number
     * of attempts, a control message signaling the error is sent to the application.
     * This function inserts sender mac address and sequence number into the sent frame. It
     * takes care of incrementing the sequence number too.
     *
     * \param destination destination mac address. set to "broadcast" for a broadcast message
     * \param msg packet to encapsulate
     * \param encapsulatedId tells the kind of encapsulated frame inside the unicast message
     * which will be used by the receiver to know the type of content, a kind of protocol ID field
     * \param priority the priority of the message, a value from 0 to 3 which will be then mapped
     * onto an AC (AC_BK = 0, ... AC_VO = 3)
     * \param channel 0 for CCH, 1 for SCH
     * \param kind id of the application, for (de)multiplexing
     */
    void sendMessageDown(int destination, cPacket* msg, int encapsulatedId, int priority, SimTime timestamp, enum Channels::ChannelNumber channel, short kind);

    /**
     * Sends an ack in response to an unicast message
     *
     * \param msg the unicast message to be acknowledged
     */
    void sendAck(const UnicastMessage* msg);

    /**
     * Resend the current unicast message after an ack timeout occurred.
     * This function also increments the number of attempts.
     */
    void resendMessage();

    /**
     * After an ack is received or a message is discarded after a certain number of attempts,
     * this method can be called to process the next packet in the queue, if any.
     */
    void processNextPacket();

public:
    UnicastProtocol();
    virtual ~UnicastProtocol();
};

#endif /* UNICASTPROTOCOL_H_ */
