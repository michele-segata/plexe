#ifndef VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_
#define VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_

#include <list>
#include <string>
#include <stdint.h>

#include "modules/mobility/traci/TraCIColor.h"
#include "modules/mobility/traci/TraCICoord.h"

namespace Veins {

class TraCIConnection;

class TraCICommandInterface
{
	public:
		TraCICommandInterface(TraCIConnection&);

		enum DepartDefs {
			DEPART_NOW = 2,
			DEPART_LANE_BEST_FREE = 5,
			DEPART_POS_BASE = 4,
			DEPART_SPEED_MAX = 3
		};

		std::pair<uint32_t, std::string> getVersion();
		void setSpeedMode(std::string nodeId, int32_t bitset);
		void setSpeed(std::string nodeId, double speed);
		void setColor(std::string nodeId, const TraCIColor& color);
		void slowDown(std::string nodeId, double speed, int time);
		void newRoute(std::string nodeId, std::string roadId);
		void setVehicleParking(std::string nodeId);
		double getEdgeCurrentTravelTime(std::string edgeId) ;
		double getEdgeMeanSpeed(std::string edgeId) ;
		std::string getEdgeId(std::string nodeId);
		std::string getCurrentEdgeOnRoute(std::string nodeId);
		std::string getLaneId(std::string nodeId);
		double getLanePosition(std::string nodeId);
		std::list<std::string> getPlannedEdgeIds(std::string nodeId);
		std::string getRouteId(std::string nodeId);
		std::list<std::string> commandGetRouteEdgeIds(std::string routeId);
		std::list<std::string> commandGetVehicleTypeIds();
		std::list<std::string> commandGetRouteIds();
		std::list<std::string> commandGetRoadIds();
		std::list<std::string> commandGetLaneIds();
		std::string commandGetLaneEdgeId(std::string laneId);
		void changeRoute(std::string nodeId, std::string roadId, double travelTime);
		double distanceRequest(const TraCICoord& position1, const TraCICoord& position2, bool returnDrivingDistance);
		void stopNode(std::string nodeId, std::string roadId, double pos, uint8_t laneid, double radius, double waittime);
		void setTrafficLightProgram(std::string trafficLightId, std::string program);
		void setTrafficLightPhaseIndex(std::string trafficLightId, int32_t index);
		std::list<std::string> getPolygonIds();
		std::string getPolygonTypeId(std::string polyId);
		std::list<TraCICoord> getPolygonShape(std::string polyId);
		void setPolygonShape(std::string polyId, const std::list<TraCICoord>& points);
		void addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<TraCICoord>& points);
		void removePolygon(std::string polyId, int32_t layer);
		void addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const TraCICoord& pos);
		void removePoi(std::string poiId, int32_t layer);
		std::list<std::string> getLaneIds();
		std::list<TraCICoord> getLaneShape(std::string laneId);
		std::string getLaneEdgeId(std::string laneId);
		double getLaneLength(std::string laneId);
		double getLaneMaxSpeed(std::string laneId);
		double getLaneMeanSpeed(std::string laneId);
		int32_t getLaneIndex(std::string nodeId);
		std::list<std::string> getJunctionIds();
		TraCICoord getJunctionPosition(std::string junctionId);
		bool addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st = -DEPART_NOW, double emitPosition = -DEPART_POS_BASE, double emitSpeed = -DEPART_SPEED_MAX, int8_t emitLane = -DEPART_LANE_BEST_FREE);
		std::string getVehicleTypeId(std::string nodeId);
		std::list<std::string> getVehicleTypeIds();
		std::list<std::string> getRouteIds();
		bool changeVehicleRoute(std::string nodeId, const std::list<std::string>& edges);
		std::pair<double, double> positionConversionLonLat(const TraCICoord&);

		/**
		 * Gets the index of the lane the vehicle is running on (0 for rightmost)
		 */
		unsigned int commandGetLaneIndex(std::string vehicleId);
		/**
		 * Gets the total number of lanes on the edge the vehicle is currently traveling
		 */
		unsigned int commandGetLanesCount(std::string vehicleId);
		/**
		 * Sets the data about the leader of the platoon. This data is usually received
		 * by means of wireless communications
		 */
		void commandSetPlatoonLeaderData(std::string vehicleId, double leaderSpeed, double leaderAcceleration, double positionX, double positionY, double time);
		/**
		 * Sets the data about the preceding vehicle in the platoon. This data is usually
		 * received by means of wireless communications
		 */
		void commandSetPrecedingVehicleData(std::string vehicleId, double speed, double acceleration, double positionX, double positionY, double time);
		/**
		 * Gets the data about a vehicle. This can be used by a platoon leader in order to query for the acceleration
		 * before sending the data to the followers
		 */
		void commandGetVehicleData(std::string vehicleId, double &speed, double &acceleration, double &controllerAcceleration, double &positionX, double &positionY, double &time);

		void commandSetGenericInformation(std::string vehicleId, int type, const void* data, int length);
		void commandGetGenericInformation(std::string vehicleId, int type, const void* params, int paramsLength, void *result);

		/**
		 * Set the cruise control desired speed
		 */
		void commandSetCruiseControlDesiredSpeed(std::string vehicleId, double desiredSpeed);
		/**
		 * Set the currently active controller, which can be either the driver, the ACC or
		 * the CACC. CC is not mentioned because CC and ACC work together
		 *
		 * @param vehicleId the id of vehicle for which the active controller must be set
		 * @param activeController the controller to be activated: 0 for driver, 1 for
		 * ACC and 2 for CACC
		 */
		void commandSetActiveController(std::string vehicleId, int activeController);

		/**
		 * Returns the currently active controller
		 */
		int commandGetActiveController(std::string vehicleId);
		/**
		 * Set CACC constant spacing
		 *
		 * @param vehicleId the id of vehicle for which the constant spacing must be set
		 * @param spacing the constant spacing in meter
		 */
		void commandSetCACCConstantSpacing(std::string vehicleId, double spacing);

		/**
		 * Returns the CACC constant spacing
		 */
		double commandGetCACCConstantSpacing(std::string vehicleId);

		/**
		 * Sets the headway time for the ACC
		 *
		 * @param vehicleId the id of the vehicle
		 * @param headway the headway time in seconds
		 */
		void commandSetACCHeadwayTime(std::string vehicleId, double headway);

		/**
		 * Enables/disables a fixed acceleration
		 *
		 * @param vehicleId the id of the vehicle
		 * @param activate activate (1) or deactivate (0) the usage of a fixed acceleration
		 * @param acceleration the fixed acceleration to be used if activate == 1
		 */
		void commandSetFixedAcceleration(std::string vehicleId, int activate, double acceleration);

		/**
		 * Returns whether a vehicle has crashed or not
		 *
		 * @param vehicleId the id of the vehicle
		 * @return true if the vehicle has crashed, false otherwise
		 */
		bool commandIsCrashed(std::string vehicleId);

		/**
		 * Tells whether the car has an ACC/CACC controller installed or not. Basically
		 * it checks the the mobility model which is driving the car
		 *
		 */
		bool commandIsCruiseControllerInstalled(std::string vehicleId);
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
		void commandSetLaneChangeAction(std::string vehicleId, int action);

		/**
		 * Returns the currently set lane change action
		 */
		int commandGetLaneChangeAction(std::string vehicleId);

		/**
		 * Set a fixed lane a car should move to
		 *
		 * @param laneIndex lane to move to, where 0 indicates the rightmost
		 */
		void commandSetFixedLane(std::string vehicleId, int laneIndex);

		/**
		 * Gets the data measured by the radar, i.e., distance and relative speed.
		 * This is basically what SUMO measures, so it gives back potentially
		 * infinite distance measurements. Taking into account that the maximum
		 * distance measurable of the Bosch LRR3 radar is 250m, when this
		 * method returns a distance value greater than 250m, it shall be
		 * interpreted like "there is nobody in front"
		 */
		void commandGetRadarMeasurements(std::string vehicleId, double &distance, double &relativeSpeed);

		void commandSetControllerFakeData(std::string vehicleId, double frontDistance, double frontSpeed, double frontAcceleration,
		                    double leaderSpeed, double leaderAcceleration);

		/**
		 * Gets the distance that a vehicle has to travel to reach the end of
		 * its route. Might be really useful for deciding when a car has to
		 * leave a platoon
		 */
		double commandGetDistanceToRouteEnd(std::string vehicleId);

		/**
		 * Gets the distance that a vehicle has traveled since the begin
		 */
		double commandGetDistanceFromRouteBegin(std::string vehicleId);

		/**
		 * Gets acceleration that the ACC has computed while the vehicle
		 * is controlled by the faked CACC
		 */
		double commandGetACCAcceleration(std::string vehicleId);


	private:
		TraCIConnection& connection;

		std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		TraCICoord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<TraCICoord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
};

}

#endif
