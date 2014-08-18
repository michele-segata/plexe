//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
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

#ifndef MOBILITY_TRACI_TRACIMOBILITY_H
#define MOBILITY_TRACI_TRACIMOBILITY_H

#include <string>
#include <fstream>
#include <list>
#include <stdexcept>

#include <BaseMobility.h>
#include "FindModule.h"
#include "mobility/traci/TraCIScenarioManager.h"

/**
 * @brief
 * Used in modules created by the TraCIScenarioManager.
 *
 * This module relies on the TraCIScenarioManager for state updates
 * and can not be used on its own.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer, David Eckhoff, Luca Bedogni, Bastian Halmos, Stefan Joerer
 *
 * @see TraCIScenarioManager
 * @see TraCIScenarioManagerLaunchd
 *
 * @ingroup mobility
 */
class TraCIMobility : public BaseMobility
{
	public:
		class Statistics {
			public:
				double firstRoadNumber; /**< for statistics: number of first road we encountered (if road id can be expressed as a number) */
				simtime_t startTime; /**< for statistics: start time */
				simtime_t totalTime; /**< for statistics: total time travelled */
				simtime_t stopTime; /**< for statistics: stop time */
				double minSpeed; /**< for statistics: minimum value of currentSpeed */
				double maxSpeed; /**< for statistics: maximum value of currentSpeed */
				double totalDistance; /**< for statistics: total distance travelled */
				double totalCO2Emission; /**< for statistics: total CO2 emission */

				void initialize();
				void watch(cSimpleModule& module);
				void recordScalars(cSimpleModule& module);
		};

		TraCIMobility() : BaseMobility(), isPreInitialized(false) {}
		virtual void initialize(int);
		virtual void finish();

		virtual void handleSelfMsg(cMessage *msg);
		virtual void preInitialize(std::string external_id, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);
		virtual void nextPosition(const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, TraCIScenarioManager::VehicleSignal signals = TraCIScenarioManager::VEH_SIGNAL_UNDEF);
		virtual void changePosition();
		virtual void updateDisplayString();
		virtual void setExternalId(std::string external_id) {
			this->external_id = external_id;
		}
		virtual std::string getExternalId() const {
			if (external_id == "") throw cRuntimeError("TraCIMobility::getExternalId called with no external_id set yet");
			return external_id;
		}
		virtual double getAntennaPositionOffset() const {
			return antennaPositionOffset;
		}
		virtual Coord getPositionAt(const simtime_t& t) const {
			return move.getPositionAt(t) ;
		}
		virtual std::string getRoadId() const {
			if (road_id == "") throw cRuntimeError("TraCIMobility::getRoadId called with no road_id set yet");
			return road_id;
		}
		virtual double getSpeed() const {
			if (speed == -1) throw cRuntimeError("TraCIMobility::getSpeed called with no speed set yet");
			return speed;
		}
		virtual TraCIScenarioManager::VehicleSignal getSignals() const {
			if (signals == -1) throw cRuntimeError("TraCIMobility::getSignals called with no signals set yet");
			return signals;
		}
		/**
		 * returns angle in rads, 0 being east, with -M_PI <= angle < M_PI.
		 */
		virtual double getAngleRad() const {
			if (angle == M_PI) throw cRuntimeError("TraCIMobility::getAngleRad called with no angle set yet");
			return angle;
		}
		virtual TraCIScenarioManager* getManager() const {
			if (!manager) manager = TraCIScenarioManagerAccess().get();
			return manager;
		}
		void commandSetSpeedMode(int32_t bitset) {
			getManager()->commandSetSpeedMode(getExternalId(), bitset);
		}
		void commandSetSpeed(double speed) {
			getManager()->commandSetSpeed(getExternalId(), speed);
		}
		void commandChangeRoute(std::string roadId, double travelTime) {
			getManager()->commandChangeRoute(getExternalId(), roadId, travelTime);
		}

		void commandNewRoute(std::string roadId) {
		    getManager()->commandNewRoute(getExternalId(), roadId);
		}
		void commandParkVehicle() {
			getManager()->commandSetVehicleParking(getExternalId());
		}

		double commandDistanceRequest(Coord position1, Coord position2, bool returnDrivingDistance) {
			return getManager()->commandDistanceRequest(position1, position2, returnDrivingDistance);
		}
		void commandStopNode(std::string roadId, double pos, uint8_t laneid, double radius, double waittime) {
			return getManager()->commandStopNode(getExternalId(), roadId, pos, laneid, radius, waittime);
		}
		std::list<std::string> commandGetPolygonIds() {
			return getManager()->commandGetPolygonIds();
		}
		std::string commandGetPolygonTypeId(std::string polyId) {
			return getManager()->commandGetPolygonTypeId(polyId);
		}
		std::list<Coord> commandGetPolygonShape(std::string polyId) {
			return getManager()->commandGetPolygonShape(polyId);
		}
		void commandSetPolygonShape(std::string polyId, std::list<Coord> points) {
			getManager()->commandSetPolygonShape(polyId, points);
		}
		bool commandAddVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st = -TraCIScenarioManager::DEPART_NOW, double emitPosition = -TraCIScenarioManager::DEPART_POS_BASE, double emitSpeed = -TraCIScenarioManager::DEPART_SPEED_MAX, int8_t emitLane = -TraCIScenarioManager::DEPART_LANE_BEST_FREE) {
			return getManager()->commandAddVehicle(vehicleId, vehicleTypeId, routeId, emitTime_st, emitPosition, emitSpeed, emitLane);
		}
		std::string commandGetVType(std::string vehicleId) {
			return getManager()->commandGetVType(vehicleId);
		}
		/**
		 * Gets the index of the lane the vehicle is running on (0 for rightmost)
		 */
		unsigned int commandGetLaneIndex(std::string vehicleId) {
		    return getManager()->commandGetLaneIndex(vehicleId);
		}
		/**
		 * Gets the total number of lanes on the edge the vehicle is currently traveling
		 */
		unsigned int commandGetLanesCount(std::string vehicleId) {
		    return getManager()->commandGetLanesCount(vehicleId);
		}
		/**
		 * Sets the data about the leader of the platoon. This data is usually received
		 * by means of wireless communications
		 */
		void commandSetPlatoonLeaderData(std::string vehicleId, double leaderSpeed, double leaderAcceleration, double positionX = 0, double positionY = 0, double time = 0) {
		    getManager()->commandSetPlatoonLeaderData(vehicleId, leaderSpeed, leaderAcceleration, positionX, positionY, time);
		}
		/**
		 * Sets the data about the preceding vehicle in the platoon. This data is usually
		 * received by means of wireless communications
		 */
		void commandSetPrecedingVehicleData(std::string vehicleId, double leaderSpeed, double leaderAcceleration, double positionX = 0, double positionY = 0, double time = 0) {
		    getManager()->commandSetPrecedingVehicleData(vehicleId, leaderSpeed, leaderAcceleration, positionX, positionY, time);
		}
		void commandSetGenericInformation(std::string vehicleId, int type, const void* data, int length) {
			getManager()->commandSetGenericInformation(vehicleId, type, data, length);
		}
		void commandGetGenericInformation(std::string vehicleId, int type, const void* params, int paramsLength, void *result) {
			getManager()->commandGetGenericInformation(vehicleId, type, params, paramsLength, result);
		}
		/**
		 * Gets the data about a vehicle. This can be used by a platoon leader in order to query for the acceleration
		 * before sending the data to the followers
		 */
		void commandGetVehicleData(std::string vehicleId, double &speed, double &acceleration, double &controllerAcceleration, double &positionX, double &positionY, double &time) {
		    return getManager()->commandGetVehicleData(vehicleId, speed, acceleration, controllerAcceleration, positionX, positionY, time);
		}
		/**
		 * Set the cruise control desired speed
		 */
		void commandSetCruiseControlDesiredSpeed(std::string vehicleId, double desiredSpeed) {
		    getManager()->commandSetCruiseControlDesiredSpeed(vehicleId, desiredSpeed);
		}
		/**
		 * Set the currently active controller, which can be either the driver, the ACC or
		 * the CACC. CC is not mentioned because CC and ACC work together
		 *
		 * @param vehicleId the id of vehicle for which the active controller must be set
		 * @param activeController the controller to be activated: 0 for driver, 1 for
		 * ACC and 2 for CACC
		 */
		void commandSetActiveController(std::string vehicleId, int activeController) {
		    getManager()->commandSetActiveController(vehicleId, activeController);
		}
		/**
		 * Returns the currently active controller
		 */
		int commandGetActiveController(std::string vehicleId) {
		    return getManager()->commandGetActiveController(vehicleId);
		}
		/**
		 * Sets the headway time for the ACC
		 *
		 * @param vehicleId the id of the vehicle
		 * @param headway the headway time in seconds
		 */
		void commandSetACCHeadwayTime(std::string vehicleId, double headway) {
			return getManager()->commandSetACCHeadwayTime(vehicleId, headway);
		}
		/**
		 * Set CACC constant spacing
		 *
		 * @param vehicleId the id of vehicle for which the constant spacing must be set
		 * @param spacing the constant spacing in meter
		 */
		void commandSetCACCConstantSpacing(std::string vehicleId, double spacing) {
			getManager()->commandSetCACCConstantSpacing(vehicleId, spacing);
		}

		/**
		 * Returns the CACC constant spacing
		 */
		double commandGetCACCConstantSpacing(std::string vehicleId) {
			return getManager()->commandGetCACCConstantSpacing(vehicleId);
		}
		/**
		 * Enables/disables a fixed acceleration
		 *
		 * @param vehicleId the id of the vehicle
		 * @param activate activate (1) or deactivate (0) the usage of a fixed acceleration
		 * @param acceleration the fixed acceleration to be used if activate == 1
		 */
		void commandSetFixedAcceleration(std::string vehicleId, int activate, double acceleration) {
			return getManager()->commandSetFixedAcceleration(vehicleId, activate, acceleration);
		}
		/**
		 * Returns whether a vehicle has crashed or not
		 *
		 * @param vehicleId the id of the vehicle
		 * @return true if the vehicle has crashed, false otherwise
		 */
		bool commandIsCrashed(std::string vehicleId) {
			return getManager()->commandIsCrashed(vehicleId);
		}
		/**
		 * Gets acceleration that the ACC has computed while the vehicle
		 * is controlled by the faked CACC
		 */
		double commandGetACCAcceleration(std::string vehicleId) {
			return getManager()->commandGetACCAcceleration(vehicleId);
		}
		/**
		 * Tells whether the car has an ACC/CACC controller installed or not. Basically
		 * it checks the the mobility model which is driving the car
		 *
		 */
		bool commandIsCruiseControllerInstalled(std::string vehicleId) {
		    return getManager()->commandIsCruiseControllerInstalled(vehicleId);
		}
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
		void commandSetLaneChangeAction(std::string vehicleId, int action) {
		    getManager()->commandSetLaneChangeAction(vehicleId, action);
		}

		int commandGetLaneChangeAction(std::string vehicleId) {
		    return getManager()->commandGetLaneChangeAction(vehicleId);
		}

		void commandSetFixedLane(std::string vehicleId, int laneIndex) {
			getManager()->commandSetFixedLane(vehicleId, laneIndex);
		}

		/**
		 * Gets the data measured by the radar, i.e., distance and relative speed.
		 * This is basically what SUMO measures, so it gives back potentially
		 * infinite distance measurements. Taking into account that the maximum
		 * distance measurable of the Bosch LRR3 radar is 250m, when this
		 * method returns a distance value greater than 250m, it shall be
		 * interpreted like "there is nobody in front"
		 */
		void commandGetRadarMeasurements(std::string vehicleId, double &distance, double &relativeSpeed) {
		    getManager()->commandGetRadarMeasurements(vehicleId, distance, relativeSpeed);
		}

		void commandSetControllerFakeData(std::string vehicleId, double frontDistance, double frontSpeed, double frontAcceleration,
		            double leaderSpeed, double leaderAcceleration) {
		    getManager()->commandSetControllerFakeData(vehicleId, frontDistance, frontSpeed, frontAcceleration, leaderSpeed, leaderAcceleration);
		}

		/**
		 * Gets the distance that a vehicle has to travel to reach the end of
		 * its route. Might be really useful for deciding when a car has to
		 * leave a platoon
		 */
		double commandGetDistanceToRouteEnd(std::string vehicleId) {
		    return getManager()->commandGetDistanceToRouteEnd(vehicleId);
		}

		/**
		 * Gets the distance that a vehicle has traveled since the beginning
		 */
		double commandGetDistanceFromRouteBegin(std::string vehicleId) {
		    return getManager()->commandGetDistanceFromRouteBegin(vehicleId);
		}

	protected:
		bool debug; /**< whether to emit debug messages */
		int accidentCount; /**< number of accidents */

		cOutVector currentPosXVec; /**< vector plotting posx */
		cOutVector currentPosYVec; /**< vector plotting posy */
		cOutVector currentSpeedVec; /**< vector plotting speed */
		cOutVector currentAccelerationVec; /**< vector plotting acceleration */
		cOutVector currentCO2EmissionVec; /**< vector plotting current CO2 emission */

		Statistics statistics; /**< everything statistics-related */

		bool isPreInitialized; /**< true if preInitialize() has been called immediately before initialize() */

		std::string external_id; /**< updated by setExternalId() */
		double antennaPositionOffset; /**< front offset for the antenna on this car */

		simtime_t lastUpdate; /**< updated by nextPosition() */
		Coord roadPosition; /**< position of front bumper, updated by nextPosition() */
		std::string road_id; /**< updated by nextPosition() */
		double speed; /**< updated by nextPosition() */
		double angle; /**< updated by nextPosition() */
		TraCIScenarioManager::VehicleSignal signals; /**<updated by nextPosition() */

		cMessage* startAccidentMsg;
		cMessage* stopAccidentMsg;
		mutable TraCIScenarioManager* manager;
		double last_speed;

		virtual void fixIfHostGetsOutside(); /**< called after each read to check for (and handle) invalid positions */

		/**
		 * Returns the amount of CO2 emissions in grams/second, calculated for an average Car
		 * @param v speed in m/s
		 * @param a acceleration in m/s^2
		 * @returns emission in g/s
		 */
		double calculateCO2emission(double v, double a) const;

		/**
		 * Calculates where the antenna of this car is, given its front bumper position
		 */
		Coord calculateAntennaPosition(const Coord& vehiclePos) const;
};

class TraCIMobilityAccess
{
	public:
		TraCIMobility* get(cModule* host) {
			TraCIMobility* traci = FindModule<TraCIMobility*>::findSubModule(host);
			ASSERT(traci);
			return traci;
		};
};


#endif

