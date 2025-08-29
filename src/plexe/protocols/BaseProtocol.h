//
// Copyright (C) 2012-2025 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef BASEPROTOCOL_H_
#define BASEPROTOCOL_H_

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"

#include "plexe/messages/PlatooningBeacon_m.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"

#include "plexe/driver/PlexeRadioDriverInterface.h"

#include <memory>
#include <tuple>

// maximum number of upper layer apps that can connect (see .ned file)
#define MAX_GATES_COUNT 10

namespace plexe {

using veins::BaseFrame1609_4;

class BaseProtocol : public veins::BaseApplLayer {

private:
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

    // map of radio interfaces from radio ids
    std::map<int, cGate*> radioOuts;

    // map of radio gates to radio interfaces type
    std::map<int, int> radioIns;

    // map of known beacons (vehicle id, sequence number)
    std::map<int, int> knownBeacons;

    // indicates whether a beacon has already been received or not
    bool isDuplicated(const PlatooningBeacon* beacon);

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
    // id range of lower radio gates
    int minRadioId, maxRadioId;

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
    cMessage* sendBeacon;
    cMessage* recordData;

    /**
     * NB: this method must be overridden by inheriting classes, BUT THEY MUST invoke the super class
     * method prior processing the message. For example, the start communication event is handled by the
     * BaseProtocol which then calls the startCommunications method. Also statistics are handled
     * by BaseProtocol and are recorder periodically.
     */
    virtual void handleSelfMsg(cMessage* msg) override;

    // TODO: implement method and pass info to upper layer (bogus platooning) as it is (msg)
    virtual void handleLowerMsg(cMessage* msg) override;

    // handle messages coming from above layers
    virtual void handleUpperMsg(cMessage* msg) override;

    // override handleMessage to manager upper layer gate array
    virtual void handleMessage(cMessage* msg) override;

    virtual void sendTo(BaseFrame1609_4* frame, enum PlexeRadioInterfaces interfaces);

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
    virtual void sendPlatooningMessage(int destinationAddress, enum PlexeRadioInterfaces interfaces = PlexeRadioInterfaces::ALL);

    virtual std::unique_ptr<BaseFrame1609_4> createBeacon(int destinationAddress);

    /**
     * This method must be overridden by subclasses to take decisions
     * about what to do.
     * Passed packet MUST NOT be freed, but just be read. Freeing is a duty of the
     * superclass
     *
     * \param pkt the platooning beacon
     * \param frame the original frame which was containing pkt
     */
    virtual void messageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* frame);

    virtual void messageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* frame, enum PlexeRadioInterfaces interface);

    /**
     * This method must be overridden by subclasses to take decisions
     * about what to do.
     * Passed packet MUST NOT be freed, but just be read. Freeing is a duty of the
     * superclass
     * Differently from the messageReceived method, this method is invoked for frames that has already been received.
     * This can happen, for example, when using multiple communication technologies or redundancy
     *
     * \param pkt the platooning beacon
     * \param frame the original frame which was containing pkt
     */
    virtual void duplicatedMessageReceived(PlatooningBeacon* pkt, BaseFrame1609_4* frame);

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
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    std::unique_ptr<traci::CommandInterface::Vehicle> plexeTraciVehicle;

public:
    // id for beacon message
    static const int BEACON_TYPE;

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

} // namespace plexe

#endif /* BASEPROTOCOL_H_ */
