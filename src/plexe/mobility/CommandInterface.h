//
// Copyright (C) 2018-2019 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/plexe.h"
#include "plexe/CC_Const.h"

#include <veins/modules/utility/HasLogProxy.h>
#include <veins/modules/mobility/traci/TraCICommandInterface.h>

#include <map>

namespace veins {
class TraCIConnection;
}

namespace plexe {
namespace traci {

class CommandInterface : public veins::HasLogProxy {
public:
    class Vehicle {
    public:
        Vehicle(CommandInterface* cifc, const std::string& nodeId)
            : cifc(cifc)
            , nodeId(nodeId)
        {
        }

        void setLaneChangeMode(int mode);
        void getLaneChangeState(int direction, int& state1, int& state2);
        void changeLane(int lane, double duration);
        /**
         * Sets the data about the leader of the platoon. This data is usually received
         * by means of wireless communications
         */
        void setLeaderVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time);
        void setPlatoonLeaderData(double leaderSpeed, double leaderAcceleration, double positionX, double positionY, double time);
        /**
         * Sets the data about the preceding vehicle in the platoon. This data is usually
         * received by means of wireless communications
         */
        void setFrontVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time);
        void setPrecedingVehicleData(double speed, double acceleration, double positionX, double positionY, double time);
        /**
         * Gets the data about a vehicle. This can be used by a platoon leader in order to query for the acceleration
         * before sending the data to the followers
         * This method is deprecated. getVehicleData with a struct parameter should be used instead
         */
        void getVehicleData(double& speed, double& acceleration, double& controllerAcceleration, double& positionX, double& positionY, double& time);

        /**
         * Gets the data about a vehicle. This can be used by a platoon leader in order to query for the acceleration
         * before sending the data to the followers
         */
        void getVehicleData(plexe::VEHICLE_DATA* data);

        /**
         * Set the cruise control desired speed
         */
        void setCruiseControlDesiredSpeed(double desiredSpeed);

        /**
         * Returns the cruise control desired speed
         */
        const double getCruiseControlDesiredSpeed();

        /**
         * Set the currently active controller, which can be either the driver, the ACC or
         * the CACC. CC is not mentioned because CC and ACC work together
         *
         * @param vehicleId the id of vehicle for which the active controller must be set
         * @param activeController the controller to be activated: 0 for driver, 1 for
         * ACC and 2 for CACC
         */
        void setActiveController(int activeController);

        /**
         * Returns the currently active controller
         */
        int getActiveController();
        /**
         * Set CACC constant spacing
         *
         * @param vehicleId the id of vehicle for which the constant spacing must be set
         * @param spacing the constant spacing in meter
         */
        void setCACCConstantSpacing(double spacing);

        /**
         * Returns the CACC constant spacing
         */
        double getCACCConstantSpacing();

        /**
         * Sets all PATH's CACC and FAKED CACC parameters. Parameters set to negative values
         * will remain untouched
         */
        void setPathCACCParameters(double omegaN = -1, double xi = -1, double c1 = -1, double distance = -1);

        /**
         * Sets all Ploeg's CACCparameters. Parameters set to negative values
         * will remain untouched
         */
        void setPloegCACCParameters(double kp = -1, double kd = -1, double h = -1);

        /**
         * Sets the headway time for the ACC
         *
         * @param vehicleId the id of the vehicle
         * @param headway the headway time in seconds
         */
        void setACCHeadwayTime(double headway);

        /**
         * Returns the headway time for the ACC
         *
         * @return double headway time
         */
        double getACCHeadwayTime();

        /**
         * Enables/disables a fixed acceleration
         *
         * @param vehicleId the id of the vehicle
         * @param activate activate (1) or deactivate (0) the usage of a fixed acceleration
         * @param acceleration the fixed acceleration to be used if activate == 1
         */
        void setFixedAcceleration(int activate, double acceleration);

        /**
         * Returns whether a vehicle has crashed or not
         *
         * @param vehicleId the id of the vehicle
         * @return true if the vehicle has crashed, false otherwise
         */
        bool isCrashed();

        /**
         * Set a fixed lane a car should move to
         *
         * @param laneIndex lane to move to, where 0 indicates the rightmost.
         * @param safe whether changing lane should respect safety distance
         * or simply avoid collisions
         * Set the lane index to -1 to give control back to the human driver
         */
        void setFixedLane(int8_t laneIndex, bool safe = false);

        /**
         * Gets the data measured by the radar, i.e., distance and relative speed.
         * This is basically what SUMO measures, so it gives back potentially
         * infinite distance measurements. Taking into account that the maximum
         * distance measurable of the Bosch LRR3 radar is 250m, when this
         * method returns a distance value greater than 250m, it shall be
         * interpreted like "there is nobody in front"
         */
        void getRadarMeasurements(double& distance, double& relativeSpeed);

        void setLeaderVehicleFakeData(double controllerAcceleration, double acceleration, double speed);
        void setLeaderFakeData(double leaderSpeed, double leaderAcceleration);

        void setFrontVehicleFakeData(double controllerAcceleration, double acceleration, double speed, double distance);
        void setFrontFakeData(double frontDistance, double frontSpeed, double frontAcceleration);

        /**
         * Gets the distance that a vehicle has to travel to reach the end of
         * its route. Might be really useful for deciding when a car has to
         * leave a platoon
         */
        double getDistanceToRouteEnd();

        /**
         * Gets the distance that a vehicle has traveled since the begin
         */
        double getDistanceFromRouteBegin();

        /**
         * Gets acceleration that the ACC has computed while the vehicle
         * is controlled by the faked CACC
         */
        double getACCAcceleration();

        /**
         * Sets data information about a vehicle in the same platoon
         */
        void setVehicleData(const struct plexe::VEHICLE_DATA* data);

        /**
         * Gets data information about a vehicle in the same platoon, as stored by this car
         */
        void getStoredVehicleData(struct plexe::VEHICLE_DATA* data, int index);

        /**
         * Determines whether PATH's and PLOEG's CACCs should use the controller
         * or the real acceleration when computing the control action
         * @param use if set to true, the vehicle will use the controller acceleration
         */
        void useControllerAcceleration(bool use);

        /**
         * If the vehicle is using the realistic engine model, this method
         * returns the current gear and the engine RPM
         * @param gear the current gear. if the realistic engine model is
         * not used, this field is set to -1
         * @param rpm the current engine rpm
         */
        void getEngineData(int& gear, double& rpm);

        /**
         * Activates or deactivates autofeeding, meaning that the user is not
         * simulating inter-vehicle communication, so the CACCs will
         * automatically take the required data from other vehicles automatically
         * @param enable: boolean to enable or disable auto feeding
         * @param leaderId: id of the leader vehicle. When disabling auto
         * feeding, this parameter can be an empty string
         * @param frontId: id of the front vehicle. When disabling auto
         * feeding, this parameter can be an empty string
         */
        void enableAutoFeed(bool enable, std::string leaderId = "", std::string frontId = "");

        /**
         * Activates or deactivates prediction, i.e., interpolation of missing
         * data for the control system
         * @param enable: enable or disable prediction
         */
        void usePrediction(bool enable);

        /**
         * Adds a platoon member to this vehicle, usually considered to be the
         * leader. Members are used to perform coordinated, whole-platoon lane
         * changes
         * @param memberId: sumo id of the member being added
         * @param position: position (0-based) of the vehicle
         */
        void addPlatoonMember(std::string memberId, int position);

        /**
         * Removes a platoon member from this vehicle, usually considered to be the
         * leader. Members are used to perform coordinated, whole-platoon lane
         * changes
         * @param memberId: sumo id of the member being removed
         */
        void removePlatoonMember(std::string memberId);

        /**
         * Enables/disables automatic, coordinated, whole-platoon lane changes.
         * This function should be invoked on the leader which decides whether
         * the platoon can gain speed by changing lane. The leader will then
         * check whether lane changing is possible and, in case, do so
         * @param enable: enable or disable automatic platoon lane changes
         */
        void enableAutoLaneChanging(bool enable);

        /**
         * Gets the total number of lanes on the edge the vehicle is currently traveling
         */
        unsigned int getLanesCount();

        veins::TraCICommandInterface::Vehicle veinsVehicle()
        {
            return {cifc->veinsCommandInterface, nodeId};
        }

    protected:
        /**
         * Tells to the CC mobility model the desired lane change action to be performed
         *
         * @param vehicleId the vehicle id to communicate the action to
         * @param action the action to be performed. this can be either:
         * 0 = driver choice: the application protocol wants to let the driver chose the lane
         * 1 = management lane: the application protocol wants the driver to move the car
         * to the management lane, i.e., the leftmost minus one
         * 2 = platooning lane: the application protocol wants the driver to move the car
         * to the platooning lane, i.e., the leftmost
         * 3 = stay there: the application protocol wants the driver to keep the car
         * into the platooning lane because the car is a part of a platoon
         */
        void setLaneChangeAction(int action);

        CommandInterface* cifc;
        const std::string nodeId;
    };

    CommandInterface(cComponent* owner, veins::TraCICommandInterface* commandInterface, veins::TraCIConnection* connection);

    void executePlexeTimestep();

    Vehicle vehicle(const std::string& nodeId)
    {
        return {this, nodeId};
    }

private:
    struct PlexeLaneChange {
        int lane;
        bool safe;
        bool wait;
    };
    using PlexeLaneChanges = std::map<std::string, PlexeLaneChange>;

    static const unsigned lca_overlapping = 1 << 13;

    void __changeLane(std::string veh, int current, int direction, bool safe = true);

    veins::TraCICommandInterface* veinsCommandInterface;
    veins::TraCIConnection* connection;
    PlexeLaneChanges laneChanges;
};

} // namespace traci
} // namespace plexe
