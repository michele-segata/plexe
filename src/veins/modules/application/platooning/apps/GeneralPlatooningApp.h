//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
// Copyright (c) 2018 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#ifndef GENERALPLATOONAPP_H_
#define GENERALPLATOONAPP_H_

#include <algorithm>
#include <memory>

#include "veins/modules/application/platooning/apps/BaseApp.h"
#include "veins/modules/application/platooning/maneuver/JoinManeuver.h"
#include "veins/modules/application/platooning/maneuver/JoinAtBack.h"

#include "veins/modules/application/platooning/messages/ManeuverMessage_m.h"

#include "veins/modules/mobility/traci/TraCIConstants.h"

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
        , role(PlatoonRole::NONE)
        , joinManeuver(nullptr)
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

    /**
     * Request start of JoinManeuver to leader
     * @param int platoonId the id of the platoon to join
     * @param int leaderId the id of the leader of the platoon to join
     * @param int position the position where the vehicle should join.
     * Depending on the implementation, this parameter might be ignored
     */
    void startJoinManeuver(int platoonId, int leaderId, int position);

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
     */
    void setInManeuver(bool b = true)
    {
        inManeuver = b;
    }

    BasePositionHelper* getPositionHelper()
    {
        return positionHelper;
    }
    Veins::TraCIMobility* getMobility()
    {
        return mobility;
    }
    Veins::TraCICommandInterface* getTraci()
    {
        return traci;
    }
    Veins::TraCICommandInterface::Vehicle* getTraciVehicle()
    {
        return traciVehicle;
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
     * Handles a UpdatePlatoonFormation in the context of this
     * application
     *
     * @param UpdatePlatoonFormation msg to handle
     */
    virtual void handleUpdatePlatoonFormation(const UpdatePlatoonFormation* msg);

    bool isJoinAllowed() const;

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

private:
    /** the role of this vehicle */
    PlatoonRole role;
    /** join maneuver implementation */
    JoinManeuver* joinManeuver;
};

#endif
