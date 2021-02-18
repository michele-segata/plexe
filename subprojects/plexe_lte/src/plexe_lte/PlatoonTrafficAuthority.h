//
// Copyright (C) 2012-2020 Michele Segata <segata@ccs-labs.org>
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

#pragma once

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/socket/SocketMap.h"

#include "inet/applications/tcpapp/TcpServerHostApp.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe_lte/messages/PlatoonUpdateMessage_m.h"
#include "plexe_lte/messages/PlatoonSearchRequest_m.h"
#include "plexe_lte/messages/PlatoonSearchResponse_m.h"
#include "plexe_lte/messages/PlatoonApproachRequest_m.h"

namespace plexe {

struct PlatoonInfo {
    int platoonId;
    int platoonLeader;
    inet::TcpSocket* socket;
    double x;
    double y;
    double speed;
};

class PlatoonTrafficAuthority : public inet::TcpServerHostApp {

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void finish() override;

    friend class PlatoonTrafficAuthorityThread;

private:
    // map from platoon id to its information
    typedef std::map<int, struct PlatoonInfo> PlatoonData;

    struct PlatoonApproach {
        // id of the platoon approaching the other
        int approachingId;
        // id of the platoon being approached
        int approachedId;
        //        // socket to communicate with the approaching platoon
        //        inet::TCPSocket *approachingSocket;
        //        // socket to communicate with the approached platoon
        //        inet::TCPSocket *approachedSocket;
    };

public:
    PlatoonTrafficAuthority();
    virtual ~PlatoonTrafficAuthority();
protected:

    PlatoonData platoonData;
    // map from approaching platoon id to id of platoon being approached
    std::map<int, int> approachingManeuvers;
    // distance from the approached platoon at which the TA stops commanding the approaching platoon
    double approachDistanceThreshold;
    // speed difference to be used to approach a platoon
    double approachSpeedDelta;

protected:

    veins::TraCICommandInterface* traci;

};

class PlatoonTrafficAuthorityThread : public inet::TcpServerThreadBase {
protected:
    PlatoonTrafficAuthority* pta = nullptr;

    /**
     * Handles a platoon update message, which includes information such as speed and position.
     * If there is an ongoing approach maneuver for the platoon sending the update (@see onPlatoonApproachRequest), updates the control command
     */
    virtual void onPlatoonUpdate(const PlatoonUpdateMessage* msg, inet::TcpSocket* socket);
    /**
     * Handles a platoon search. A platoon might search for a nearby platoon to merge with.
     * The request includes a simple criterion, i.e., find the platoon that is the closest in terms of distance or of relative speed
     */
    virtual void onPlatoonSearch(const PlatoonSearchRequest* msg);
    /**
     * Handles a request for approaching a platoon.
     * The traffic authority instantiates a new approach maneuver and then periodically (every platoon update) sends speed commands to get the approaching platoon closer to the approached one.
     * This procedure continues until the two platoons are closer than the approachDistanceThreshold parameter.
     * At this point, the control is given to the approaching platoon, which autonomously approaches the other platoon
     */
    virtual void onPlatoonApproachRequest(const PlatoonApproachRequest* msg);

    /**
     * Sends the result of a platoon search back to the requester.
     *
     * @param searchingPlatoon the platoon that sent the request
     * @param matchingPlatoon the platoon matches the criterion of the requester
     * @param distance the distance at which the platoon is located
     * @param speedDifference the relative speed between the searching and the matching platoon
     * @param ahead whether the searching platoon is ahead or behind the matching platoon
     */
    void sendPlatoonSearchResponse(const PlatoonInfo& searchingPlatoon, const PlatoonInfo& matchingPlatoon, double distance, double speedDifference, bool ahead);
    /**
     * Sends a command to a platoon to change its speed
     * @param platoon the platoon to which the command should be sent
     * @param speed the speed to be set
     */
    void sendSpeedCommand(const PlatoonInfo& platoon, double speed);

    /**
     * Tells an approaching platoon to directly contact the approached one and coordinate the merging process autonomously
     */
    void sendContactPlatoonCommand(const PlatoonInfo& platoon, int contactPlatoonId, int contactLeaderId);

    /**
     * Utility function used to populate a message with common fields
     */
    void populateResponse(PlatoonTAQuery& msg, const PlatoonInfo& destination);

    /**
     * Computes what a platoon should do to approach another platoon.
     * If this function finds the platoon to be farther away than the approachDistanceThreshold parameter, it will instruct the approaching platoon to accelerate or slow down to get closed to the approached platoon.
     * If the two platoons are instead closer than the threshold, it stops controlling the approaching platoon and tells it to autonomously merge with the other
     */
    void computeApproachAction(int approachingId, int approachedId);
    /**
     * Computes the driving distance between two platoons
     * @param first first platoon
     * @param second second platoon
     * @param ahead boolean variable that will store whether first is ahead of second or not
     * @return the distance between first and second
     */
    double getDistance(const PlatoonInfo& first, const PlatoonInfo& second, bool& ahead);

public:
    /**
     * Called when connection is established.
     */
    virtual void established() override;

    /*
     * Called when a data packet arrives. To be redefined.
     */
    virtual void dataArrived(inet::Packet* packet, bool urgent) override;

    /*
     * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
     */
    virtual void timerExpired(cMessage* timer) override;

    virtual void init(inet::TcpServerHostApp* hostmodule, inet::TcpSocket* socket) override { TcpServerThreadBase::init(hostmodule, socket); pta = check_and_cast<PlatoonTrafficAuthority*>(hostmod); }

    void sendInetPacket(inet::TcpSocket* socketc, cPacket* packet);
};

} // namespace plexe
