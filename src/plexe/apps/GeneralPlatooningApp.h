//
// Copyright (C) 2012-2022 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2022 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#ifndef GENERALPLATOONAPP_H_
#define GENERALPLATOONAPP_H_

#include <algorithm>
#include <memory>

#include "plexe/apps/BaseApp.h"
#include "plexe/maneuver/JoinManeuver.h"
#include "plexe/maneuver/JoinAtBack.h"
#include "plexe/maneuver/MergeAtBack.h"

#include "plexe/messages/ManeuverMessage_m.h"
#include "plexe/messages/UpdatePlatoonData_m.h"

#include "plexe/scenarios/BaseScenario.h"

#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/utility/SignalManager.h"

namespace plexe {

/** possible roles of this vehicle */
enum class PlatoonRole : size_t {
    NONE, ///< The vehicle is not in a Platoon
    LEADER, ///< The vehicle is the leader of its Platoon
    FOLLOWER, ///< The vehicle is a normal follower in its Platoon
    JOINER ///< The vehicle is in the process of joining a Platoon
};

/**
 * General purpose application for Platoons
 *
 * This class handles maintenance of an existing Platoon by storing relevant
 * information about it and feeding the CACC.
 * It also provides an easily extendable base for supporting arbitrary
 * maneuvers.
 * Currently, it already contains functionality for the join maneuver.
 *
 * @see BaseApp
 * @see Maneuver
 * @see JoinManeuver
 * @see JoinAtBack
 * @see ManeuverMessage
 */
class GeneralPlatooningApp : public BaseApp {

public:
    /** c'tor for GeneralPlatooningApp */
    GeneralPlatooningApp()
        : inManeuver(false)
        , activeManeuver(nullptr)
        , scenario(nullptr)
        , role(PlatoonRole::NONE)
        , joinManeuver(nullptr)
        , mergeManeuver(nullptr)
    {
    }

    /** d'tor for GeneralPlatooningApp */
    virtual ~GeneralPlatooningApp();

    /**
     * Returns the role of this car in the platoon
     *
     * @return PlatoonRole the role in the platoon
     * @see PlatoonRole
     */
    const PlatoonRole& getPlatoonRole() const
    {
        return role;
    }

    /**
     * Sets the role of this car in the platoon
     *
     * @param PlatoonRole r the role in the platoon
     * @see PlatoonRole
     */
    void setPlatoonRole(PlatoonRole r);

    /** override from BaseApp */
    virtual void initialize(int stage) override;

    /** override from BaseApp */
    virtual void handleSelfMsg(cMessage* msg) override;

    /**
     * Request start of JoinManeuver to leader
     * @param int platoonId the id of the platoon to join
     * @param int leaderId the id of the leader of the platoon to join
     * @param int position the position where the vehicle should join.
     * Depending on the implementation, this parameter might be ignored
     */
    void startJoinManeuver(int platoonId, int leaderId, int position);

    /**
     * Request start of MergeManeuver to leader
     * @param int platoonId the id of the platoon to join
     * @param int leaderId the id of the leader of the platoon to join
     * @param int position the position where the vehicle should join.
     * Depending on the implementation, this parameter might be ignored
     */
    void startMergeManeuver(int platoonId, int leaderId, int position);

    /** Abort join maneuver */
    void abortJoinManeuver();

    /**
     * Returns whether this car is in a maneuver
     * @return bool true, if this car is in a maneuver, else false
     */
    bool isInManeuver() const
    {
        return inManeuver;
    }

    /**
     * Set whether this car is in a maneuver
     * @param bool b whether this car is in a maneuver
     * @param maneuver the maneuver that is currently active
     */
    void setInManeuver(bool b, Maneuver* maneuver)
    {
        inManeuver = b;
        if (inManeuver)
            activeManeuver = maneuver;
        else
            activeManeuver = nullptr;
    }

    BasePositionHelper* getPositionHelper()
    {
        return positionHelper;
    }

    veins::TraCIMobility* getMobility()
    {
        return mobility;
    }

    veins::TraCICommandInterface* getTraci()
    {
        return traci;
    }

    veins::TraCICommandInterface::Vehicle* getTraciVehicle()
    {
        return traciVehicle;
    }

    traci::CommandInterface* getPlexeTraci()
    {
        return plexeTraci;
    }

    traci::CommandInterface::Vehicle* getPlexeTraciVehicle()
    {
        return plexeTraciVehicle.get();
    }

    /**
     * Sends a unicast message
     *
     * @param cPacket msg message to be encapsulated into the unicast
     * message
     * @param int destination of the message
     */
    virtual void sendUnicast(cPacket* msg, int destination);

    /**
     * Fills members of a ManeuverMessage
     *
     * @param msg ManeuverMessage the message to be filled
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     */
    void fillManeuverMessage(ManeuverMessage* msg, int vehicleId, std::string externalId, int platoonId, int destinationId);

    /**
     * Creates a UpdatePlatoonFormation message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int platoonSpeed the speed of the platoon
     * @param int platoonLane the id of the lane of the platoon
     * @param std::vector<int> platoonFormation the new platoon formation
     */
    UpdatePlatoonFormation* createUpdatePlatoonFormation(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& platoonFormation);

    /**
     * Creates a UpdatePlatoonData message
     *
     * @param int vehicleId the id of the sending vehicle
     * @param int platoonId the id of the platoon of the sending vehicle
     * @param int destinationId the id of the destination
     * @param int platoonSpeed the speed of the platoon
     * @param int platoonLane the id of the lane of the platoon
     * @param std::vector<int> platoonFormation the new platoon formation
     * @param int newPlatoonId the new platoon id to be set
     */
    UpdatePlatoonData* createUpdatePlatoonData(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& platoonFormation, int newPlatoonId);

    /**
     * Handles a UpdatePlatoonData in the context of this
     * application
     *
     * @param UpdatePlatoonData msg to handle
     */
    virtual void handleUpdatePlatoonData(const UpdatePlatoonData* msg);

    /**
     * Handles a UpdatePlatoonFormation in the context of this
     * application
     *
     * @param UpdatePlatoonFormation msg to handle
     */
    virtual void handleUpdatePlatoonFormation(const UpdatePlatoonFormation* msg);

    bool isJoinAllowed() const;

    /**
     * Returns the controller that has been chosen for the scenario
     */
    enum ACTIVE_CONTROLLER getController();

    /**
     * Returns the inter-vehicle distance for the chosen controller and the given speed
     */
    double getTargetDistance(double speed);

protected:
    /** override this method of BaseApp. we want to handle it ourself */
    virtual void handleLowerMsg(cMessage* msg) override;

    /**
     * Handles PlatoonBeacons
     *
     * @param PlatooningBeacon pb to handle
     */
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) override;

    /**
     * Handles ManeuverMessages
     *
     * @param ManeuverMessage mm to handle
     */
    virtual void onManeuverMessage(ManeuverMessage* mm);

    /** am i in a maneuver? */
    bool inManeuver;
    /** which maneuver is currently active? */
    Maneuver* activeManeuver;

    /** used to receive the "retries exceeded" signal **/
    virtual void receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details) override;

    /** used by maneuvers to schedule self messages, as they are not omnet modules */
    virtual void scheduleSelfMsg(simtime_t t, cMessage* msg);

    BaseScenario* scenario;

private:
    /** the role of this vehicle */
    PlatoonRole role;
    /** join maneuver implementation */
    JoinManeuver* joinManeuver;
    /** platoons merge maneuver implementation */
    JoinManeuver* mergeManeuver;
};

} // namespace plexe

#endif
