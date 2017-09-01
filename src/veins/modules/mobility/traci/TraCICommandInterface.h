#ifndef VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_
#define VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_

#include <list>
#include <string>
#include <stdint.h>

#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/base/utils/Coord.h"

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

		// General methods that do not deal with a particular object in the simulation
		std::pair<uint32_t, std::string> getVersion();
		std::pair<double, double> getLonLat(const Coord&);
		double getDistance(const Coord& position1, const Coord& position2, bool returnDrivingDistance);

		// Vehicle methods
		bool addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st = -DEPART_NOW, double emitPosition = -DEPART_POS_BASE, double emitSpeed = -DEPART_SPEED_MAX, int8_t emitLane = -DEPART_LANE_BEST_FREE);
		class Vehicle {
			public:
				Vehicle(TraCICommandInterface* traci, std::string nodeId) : traci(traci), nodeId(nodeId) {
					connection = &traci->connection;
				}

				void setSpeedMode(int32_t bitset);
				void setSpeed(double speed);
				void setColor(const TraCIColor& color);
				void slowDown(double speed, int time);
				void newRoute(std::string roadId);
				void setParking();
				std::string getRoadId();
				std::string getCurrentRoadOnRoute();
				std::string getLaneId();
				double getLanePosition();
				std::list<std::string> getPlannedRoadIds();
				std::string getRouteId();
				void changeRoute(std::string roadId, double travelTime);
				void stopAt(std::string roadId, double pos, uint8_t laneid, double radius, double waittime);
				int32_t getLaneIndex();
				std::string getTypeId();
				bool changeVehicleRoute(const std::list<std::string>& roads);
				void setParameter(const std::string &parameter, int value);
				void setParameter(const std::string &parameter, double value);
				void setParameter(const std::string &parameter, const std::string &value);
				/**
				 * Gets the total number of lanes on the edge the vehicle is currently traveling
				 */
				unsigned int getLanesCount();
				/**
				 * Sets the data about the leader of the platoon. This data is usually received
				 * by means of wireless communications
				 */
				void setPlatoonLeaderData(double leaderSpeed, double leaderAcceleration, double positionX, double positionY, double time);
				/**
				 * Sets the data about the preceding vehicle in the platoon. This data is usually
				 * received by means of wireless communications
				 */
				void setPrecedingVehicleData(double speed, double acceleration, double positionX, double positionY, double time);
				/**
				 * Gets the data about a vehicle. This can be used by a platoon leader in order to query for the acceleration
				 * before sending the data to the followers
				 */
				void getVehicleData(double &speed, double &acceleration, double &controllerAcceleration, double &positionX, double &positionY, double &time);

				/**
				 * Set the cruise control desired speed
				 */
				void setCruiseControlDesiredSpeed(double desiredSpeed);
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
				 * Sets the headway time for the ACC
				 *
				 * @param vehicleId the id of the vehicle
				 * @param headway the headway time in seconds
				 */
				void setACCHeadwayTime(double headway);

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
				 * Tells whether the car has an ACC/CACC controller installed or not. Basically
				 * it checks the the mobility model which is driving the car
				 *
				 */
				bool isCruiseControllerInstalled();

				/**
				 * Set a fixed lane a car should move to
				 *
				 * @param laneIndex lane to move to, where 0 indicates the rightmost.
				 * Set the lane index to -1 to give control back to the human driver
				 */
				void setFixedLane(int8_t laneIndex);

				/**
				 * Gets the data measured by the radar, i.e., distance and relative speed.
				 * This is basically what SUMO measures, so it gives back potentially
				 * infinite distance measurements. Taking into account that the maximum
				 * distance measurable of the Bosch LRR3 radar is 250m, when this
				 * method returns a distance value greater than 250m, it shall be
				 * interpreted like "there is nobody in front"
				 */
				void getRadarMeasurements(double &distance, double &relativeSpeed);

				void setControllerFakeData(double frontDistance, double frontSpeed, double frontAcceleration,
				                    double leaderSpeed, double leaderAcceleration);

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
				 * Returns the vehicle type of a vehicle
				 */
				std::string getVType();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string nodeId;

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
		};
		Vehicle vehicle(std::string nodeId) {
			return Vehicle(this, nodeId);
		}

		// Road methods
		std::list<std::string> getRoadIds();
		class Road {
			public:
				Road(TraCICommandInterface* traci, std::string roadId) : traci(traci), roadId(roadId) {
					connection = &traci->connection;
				}

				double getCurrentTravelTime();
				double getMeanSpeed();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string roadId;
		};
		Road road(std::string roadId) {
			return Road(this, roadId);
		}

		// Lane methods
		std::list<std::string> getLaneIds();
		class Lane {
			public:
				Lane(TraCICommandInterface* traci, std::string laneId) : traci(traci), laneId(laneId) {
					connection = &traci->connection;
				}

				std::list<Coord> getShape();
				std::string getRoadId();
				double getLength();
				double getMaxSpeed();
				double getMeanSpeed();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string laneId;
		};
		Lane lane(std::string laneId) {
			return Lane(this, laneId);
		}

		// Trafficlight methods
		class Trafficlight {
			public:
				Trafficlight(TraCICommandInterface* traci, std::string trafficLightId) : traci(traci), trafficLightId(trafficLightId) {
					connection = &traci->connection;
				}

				void setProgram(std::string program);
				void setPhaseIndex(int32_t index);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string trafficLightId;
		};
		Trafficlight trafficlight(std::string trafficLightId) {
			return Trafficlight(this, trafficLightId);
		}

		// Polygon methods
		std::list<std::string> getPolygonIds();
		void addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points);
		class Polygon {
			public:
				Polygon(TraCICommandInterface* traci, std::string polyId) : traci(traci), polyId(polyId) {
					connection = &traci->connection;
				}

				std::string getTypeId();
				std::list<Coord> getShape();
				void setShape(const std::list<Coord>& points);
				void remove(int32_t layer);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string polyId;
		};
		Polygon polygon(std::string polyId) {
			return Polygon(this, polyId);
		}

		// Poi methods
		void addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos);
		class Poi {
			public:
				Poi(TraCICommandInterface* traci, std::string poiId) : traci(traci), poiId(poiId) {
					connection = &traci->connection;
				}

				void remove(int32_t layer);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string poiId;
		};
		Poi poi(std::string poiId) {
			return Poi(this, poiId);
		}

		// Junction methods
		std::list<std::string> getJunctionIds();
		class Junction {
			public:
				Junction(TraCICommandInterface* traci, std::string junctionId) : traci(traci), junctionId(junctionId) {
					connection = &traci->connection;
				}

				Coord getPosition();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string junctionId;
		};
		Junction junction(std::string junctionId) {
			return Junction(this, junctionId);
		}

		// Route methods
		std::list<std::string> getRouteIds();
		class Route {
			public:
				Route(TraCICommandInterface* traci, std::string routeId) : traci(traci), routeId(routeId) {
					connection = &traci->connection;
				}

				std::list<std::string> getRoadIds();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string routeId;
		};
		Route route(std::string routeId) {
			return Route(this, routeId);
		}

		// Vehicletype methods
		std::list<std::string> getVehicleTypeIds();

		// GuiView methods
		class GuiView {
			public:
				GuiView(TraCICommandInterface* traci, std::string viewId) : traci(traci), viewId(viewId) {
					connection = &traci->connection;
				}

				void setScheme(std::string name);
				void setZoom(double zoom);
				void setBoundary(Coord p1, Coord p2);
				void takeScreenshot(std::string filename = "");
				void trackVehicle(std::string vehicleId);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string viewId;
		};
		GuiView guiView(std::string viewId) {
			return GuiView(this, viewId);
		}


	private:
		TraCIConnection& connection;

		std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		Coord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<Coord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
};

}

#endif
