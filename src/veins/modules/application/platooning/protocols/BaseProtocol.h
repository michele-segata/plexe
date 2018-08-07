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

#ifndef BASEPROTOCOL_H_
#define BASEPROTOCOL_H_

#include "veins/base/modules/BaseApplLayer.h"

#include "veins/modules/application/platooning/UnicastProtocol.h"
#include "veins/modules/application/platooning/messages/PlatooningBeacon_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

#include <tuple>

// maximum number of upper layer apps that can connect (see .ned file)
#define MAX_GATES_COUNT 10

class BaseProtocol : public Veins::BaseApplLayer {

private:
    // signals for busy channel and collisions
    static const simsignalwrap_t sigChannelBusy;
    static const simsignalwrap_t sigCollision;

    // amount of time channel has been observed busy during the last "statisticsPeriod" seconds
    SimTime busyTime;
    // count the number of collision at the phy layer
    int nCollisions;
    // time at which channel turned busy
    SimTime startBusy;
    // indicates whether channel is busy or not
    bool channelBusy;

    // record the delay between each pair of messages received from leader and car in front
    SimTime lastLeaderMsgTime;
    SimTime lastFrontMsgTime;

    // own id for statistics
    cOutVector nodeIdOut;

    // output vectors for busy time and collisions
    cOutVector busyTimeOut, collisionsOut;

    // output vector for delays
    cOutVector leaderDelayIdOut, frontDelayIdOut, leaderDelayOut, frontDelayOut;

protected:
    // determines position and role of each vehicle
    BasePositionHelper* positionHelper;

    // id of this vehicle
    int myId;
    // sequence number of sent messages
    int seq_n;
    // vehicle length
    double length;

    // beaconing interval (i.e., update frequency)
    SimTime beaconingInterval;
    // priority used for messages (i.e., the access category)
    int priority;
    // packet size of the platooning message
    int packetSize;

    // input/output gates from/to upper layer
    int upperControlIn, upperControlOut, lowerLayerIn, lowerLayerOut;
    // id range of input gates from upper layer
    int minUpperId, maxUpperId, minUpperControlId, maxUpperControlId;

    // registered upper layer applications. this is a mapping between
    // beacon id inside packets coming from upper layer and the gate they
    // the application is connected to. convention: id, from app, to app
    typedef cGate OutputGate;
    typedef cGate InputGate;
    typedef cGate ControlInputGate;
    typedef cGate ControlOutputGate;
    typedef std::tuple<InputGate*, OutputGate*, ControlInputGate*, ControlOutputGate*> AppInOut;
    typedef std::vector<AppInOut> AppList;
    typedef std::map<int, AppList> ApplicationMap;
    ApplicationMap apps;
    // number of gates from the array used
    int usedGates;
    // maps of already existing connections
    typedef cGate ThisGate;
    typedef cGate OtherGate;
    typedef std::map<OtherGate*, ThisGate*> GateConnections;
    GateConnections connections;

    // messages for scheduleAt
    cMessage *sendBeacon, *recordData;

    /**
     * NB: this method must be overridden by inheriting classes, BUT THEY MUST invoke the super class
     * method prior processing the message. For example, the start communication event is handled by the
     * BaseProtocol which then calls the startCommunications method. Also statistics are handled
     * by BaseProtocol and are recorder periodically.
     */
    virtual void handleSelfMsg(cMessage* msg) override;

    // TODO: implement method and pass info to upper layer (bogus platooning) as it is (msg)
    virtual void handleLowerMsg(cMessage* msg) override;

    // handle unicast messages coming from above layers
    virtual void handleUpperMsg(cMessage* msg) override;

    // handle control messages coming from above
    virtual void handleUpperControl(cMessage* msg) override;

    // handle control messages coming from below
    virtual void handleLowerControl(cMessage* msg) override;

    // handles and application layer message
    void handleUnicastMsg(UnicastMessage* unicast);

    // override handleMessage to manager upper layer gate array
    virtual void handleMessage(cMessage* msg) override;

    // signal handler
    using BaseApplLayer::receiveSignal;
    void receiveSignal(cComponent* source, simsignal_t signalID, bool v, cObject* details) override;
    void receiveSignal(cComponent* source, simsignal_t signalID, bool v)
    {
        receiveSignal(source, signalID, v, 0);
    }

    /**
     * Sends a platooning message with all information about the car. This is an utility function for
     * subclasses
     */
    void sendPlatooningMessage(int destinationAddress);

    /**
     * This method must be overridden by subclasses to take decisions
     * about what to do.
     * Passed packet MUST NOT be freed, but just be read. Freeing is a duty of the
     * superclass
     *
     * \param pkt the platooning beacon
     * \param unicast the original unicast packet which was containing pkt
     */
    virtual void messageReceived(PlatooningBeacon* pkt, UnicastMessage* unicast);

    /**
     * These methods signal changes in channel busy status to subclasses
     * or occurrences of collisions.
     * Subclasses which are interested should ovverride these methods.
     */
    virtual void channelBusyStart()
    {
    }
    virtual void channelIdleStart()
    {
    }
    virtual void collision()
    {
    }

    // traci mobility. used for getting/setting info about the car
    Veins::TraCIMobility* mobility;
    Veins::TraCICommandInterface* traci;
    Veins::TraCICommandInterface::Vehicle* traciVehicle;

public:
    // id for beacon message
    static const int BEACON_TYPE = 12345;

    BaseProtocol()
    {
        sendBeacon = nullptr;
        recordData = nullptr;
        usedGates = 0;
    }
    virtual ~BaseProtocol();

    virtual void initialize(int stage) override;

    // register a higher level application by its id
    void registerApplication(int applicationId, InputGate* appInputGate, OutputGate* appOutputGate, ControlInputGate* appControlInputGate, ControlOutputGate* appControlOutputGate);
};

#endif /* BASEPROTOCOL_H_ */
