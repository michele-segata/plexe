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

#ifndef WORLD_TRACI_TRACISCENARIOMANAGER_H
#define WORLD_TRACI_TRACISCENARIOMANAGER_H

#include <map>
#include <list>
#include <sstream>
#include <iomanip>

#include <omnetpp.h>

#include "Coord.h"
#include "BaseWorldUtility.h"
#include "BaseConnectionManager.h"
#include "FindModule.h"

/**
 * @brief
 * Creates and moves nodes controlled by a TraCI server.
 *
 * If the server is a SUMO road traffic simulation, you can use the
 * TraCIScenarioManagerLaunchd module and sumo-launchd.py script instead.
 *
 * All nodes created thus must have a TraCIMobility submodule.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer, David Eckhoff, Falko Dressler, Zheng Yao, Tobias Mayer, Alvaro Torres Cortes, Luca Bedogni
 *
 * @see TraCIMobility
 * @see TraCIScenarioManagerLaunchd
 *
 */
class TraCIScenarioManager : public cSimpleModule
{
	public:
		/**
		 * TraCI compatible color container
		 */
		struct Color {
			Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) : red(red), green(green), blue(blue), alpha(alpha) {}
			uint8_t red;
			uint8_t green;
			uint8_t blue;
			uint8_t alpha;
		};

		enum VehicleSignal {
			VEH_SIGNAL_UNDEF = -1,
			VEH_SIGNAL_NONE = 0,
			VEH_SIGNAL_BLINKER_RIGHT = 1,
			VEH_SIGNAL_BLINKER_LEFT = 2,
			VEH_SIGNAL_BLINKER_EMERGENCY = 4,
			VEH_SIGNAL_BRAKELIGHT = 8,
			VEH_SIGNAL_FRONTLIGHT = 16,
			VEH_SIGNAL_FOGLIGHT = 32,
			VEH_SIGNAL_HIGHBEAM = 64,
			VEH_SIGNAL_BACKDRIVE = 128,
			VEH_SIGNAL_WIPER = 256,
			VEH_SIGNAL_DOOR_OPEN_LEFT = 512,
			VEH_SIGNAL_DOOR_OPEN_RIGHT = 1024,
			VEH_SIGNAL_EMERGENCY_BLUE = 2048,
			VEH_SIGNAL_EMERGENCY_RED = 4096,
			VEH_SIGNAL_EMERGENCY_YELLOW = 8192
		};

		~TraCIScenarioManager();
		virtual int numInitStages() const { return std::max(cSimpleModule::numInitStages(), 2); }
		virtual void initialize(int stage);
		virtual void finish();
		virtual void handleMessage(cMessage *msg);
		virtual void handleSelfMsg(cMessage *msg);

		std::pair<uint32_t, std::string> commandGetVersion();
		void commandSetSpeedMode(std::string nodeId, int32_t bitset);
		void commandSetSpeed(std::string nodeId, double speed);
		void commandNewRoute(std::string nodeId, std::string roadId);
		void commandSetVehicleParking(std::string nodeId);
		std::string commandGetEdgeId(std::string nodeId);
		std::string commandGetCurrentEdgeOnRoute(std::string nodeId);
		std::string commandGetLaneId(std::string nodeId);
		double commandGetLanePosition(std::string nodeId);
		std::list<std::string> commandGetPlannedEdgeIds(std::string nodeId);
		std::string commandGetRouteId(std::string nodeId);
		std::list<std::string> commandGetRouteEdgeIds(std::string routeId);
		void commandChangeRoute(std::string nodeId, std::string roadId, double travelTime);
		double commandDistanceRequest(Coord position1, Coord position2, bool returnDrivingDistance);
		void commandStopNode(std::string nodeId, std::string roadId, double pos, uint8_t laneid, double radius, double waittime);
		void commandSetTrafficLightProgram(std::string trafficLightId, std::string program);
		void commandSetTrafficLightPhaseIndex(std::string trafficLightId, int32_t index);
		std::list<std::string> commandGetPolygonIds();
		std::string commandGetPolygonTypeId(std::string polyId);
		std::list<Coord> commandGetPolygonShape(std::string polyId);
		void commandSetPolygonShape(std::string polyId, std::list<Coord> points);
		void commandAddPolygon(std::string polyId, std::string polyType, const Color& color, bool filled, int32_t layer, std::list<Coord> points);
		std::list<std::string> commandGetLaneIds();
		std::list<Coord> commandGetLaneShape(std::string laneId);
		std::string commandGetLaneEdgeId(std::string laneId);
		double commandGetLaneLength(std::string laneId);
		double commandGetLaneMaxSpeed(std::string laneId);
		double commandGetLaneMeanSpeed(std::string laneId);
		std::list<std::string> commandGetJunctionIds();
		Coord commandGetJunctionPosition(std::string junctionId);
		bool commandAddVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, std::string laneId, float emitPosition, float emitSpeed);
		std::string commandGetVType(std::string vehicleId);

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


		const std::map<std::string, cModule*>& getManagedHosts() {
			return hosts;
		}

	protected:
		/**
		 * Coord equivalent for storing TraCI coordinates
		 */
		struct TraCICoord {
			TraCICoord() : x(0), y(0) {}
			TraCICoord(double x, double y) : x(x), y(y) {}
			double x;
			double y;
		};

		/**
		 * Byte-buffer that stores values in TraCI byte-order
		 */
		class TraCIBuffer {
			public:
				TraCIBuffer() : buf() {
					buf_index = 0;
				}

				TraCIBuffer(std::string buf) : buf(buf) {
					buf_index = 0;
				}

				template<typename T> T read() {
					T buf_to_return;
					unsigned char *p_buf_to_return = reinterpret_cast<unsigned char*>(&buf_to_return);

					if (isBigEndian()) {
						for (size_t i=0; i<sizeof(buf_to_return); ++i) {
							if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
							p_buf_to_return[i] = buf[buf_index++];
						}
					} else {
						for (size_t i=0; i<sizeof(buf_to_return); ++i) {
							if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
							p_buf_to_return[sizeof(buf_to_return)-1-i] = buf[buf_index++];
						}
					}

					return buf_to_return;
				}

				void writeBuffer(const unsigned char *buffer, size_t size) {
					if (isBigEndian()) {
						for (size_t i=0; i<size; ++i) {
							buf += buffer[i];
						}
					} else {
						for (size_t i=0; i<size; ++i) {
							buf += buffer[size-1-i];
						}
					}
				}

				void readBuffer(unsigned char *buffer, size_t size) {
					if (isBigEndian()) {
						for (size_t i=0; i<size; ++i) {
							if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
							buffer[i] = buf[buf_index++];
						}
					} else {
						for (size_t i=0; i<size; ++i) {
							if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
							buffer[size-1-i] = buf[buf_index++];
						}
					}
				}

				template<typename T> void write(T inv) {
					unsigned char *p_buf_to_send = reinterpret_cast<unsigned char*>(&inv);

					if (isBigEndian()) {
						for (size_t i=0; i<sizeof(inv); ++i) {
							buf += p_buf_to_send[i];
						}
					} else {
						for (size_t i=0; i<sizeof(inv); ++i) {
							buf += p_buf_to_send[sizeof(inv)-1-i];
						}
					}
				}

				template<typename T> T read(T& out) {
					out = read<T>();
					return out;
				}

				template<typename T> TraCIBuffer& operator >>(T& out) {
					out = read<T>();
					return *this;
				}

				template<typename T> TraCIBuffer& operator <<(const T& inv) {
					write(inv);
					return *this;
				}

				bool eof() const {
					return buf_index == buf.length();
				}

				void set(std::string buf) {
					this->buf = buf;
					buf_index = 0;
				}

				void clear() {
					set("");
				}

				std::string str() const {
					return buf;
				}

				std::string hexStr() const {
					std::stringstream ss;
					for (std::string::const_iterator i = buf.begin() + buf_index; i != buf.end(); ++i) {
						if (i != buf.begin()) ss << " ";
						ss << std::hex << std::setw(2) << std::setfill('0') << (int)(uint8_t)*i;
					}
					return ss.str();
				}

			protected:
				bool isBigEndian() {
					short a = 0x0102;
					unsigned char *p_a = reinterpret_cast<unsigned char*>(&a);
					return (p_a[0] == 0x01);
				}

				std::string buf;
				size_t buf_index;
		};

		bool debug; /**< whether to emit debug messages */
		simtime_t connectAt; /**< when to connect to TraCI server (must be the initial timestep of the server) */
		simtime_t firstStepAt; /**< when to start synchronizing with the TraCI server (-1: immediately after connecting) */
		simtime_t updateInterval; /**< time interval of hosts' position updates */
		std::string moduleType; /**< module type to be used in the simulation for each managed vehicle */
		std::string moduleName; /**< module name to be used in the simulation for each managed vehicle */
		std::string moduleDisplayString; /**< module displayString to be used in the simulation for each managed vehicle */
		std::string host;
		int port;
		bool autoShutdown; /**< Shutdown module as soon as no more vehicles are in the simulation */
		int margin;
		double penetrationRate;
		std::list<std::string> roiRoads; /**< which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty */
		std::list<std::pair<TraCICoord, TraCICoord> > roiRects; /**< which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty */

		void* socketPtr;
		TraCICoord netbounds1; /* network boundaries as reported by TraCI (x1, y1) */
		TraCICoord netbounds2; /* network boundaries as reported by TraCI (x2, y2) */

		size_t nextNodeVectorIndex; /**< next OMNeT++ module vector index to use */
		std::map<std::string, cModule*> hosts; /**< vector of all hosts managed by us */
		std::set<std::string> unEquippedHosts;
		std::set<std::string> subscribedVehicles; /**< all vehicles we have already subscribed to */
		uint32_t activeVehicleCount; /**< number of vehicles reported as active by TraCI server */
		bool autoShutdownTriggered;
		cMessage* connectAndStartTrigger; /**< self-message scheduled for when to connect to TraCI server and start running */
		cMessage* executeOneTimestepTrigger; /**< self-message scheduled for when to next call executeOneTimestep */

		BaseWorldUtility* world;
		BaseConnectionManager* cc;

		uint32_t getCurrentTimeMs(); /**< get current simulation time (in ms) */

		void executeOneTimestep(); /**< read and execute all commands for the next timestep */

		void connect();
		virtual void init_traci();

		void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);
		cModule* getManagedModule(std::string nodeId); /**< returns a pointer to the managed module named moduleName, or 0 if no module can be found */
		void deleteModule(std::string nodeId);

		bool isModuleUnequipped(std::string nodeId); /**< returns true if this vehicle is Unequipped */

		/**
		 * returns whether a given position lies within the simulation's region of interest.
		 * Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI
		 */
		bool isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle);

		/**
		 * sends a single command via TraCI, checks status response, returns additional responses
		 */
		TraCIBuffer queryTraCI(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

		/**
		 * sends a single command via TraCI, expects no reply, returns true if successful
		 */
		TraCIScenarioManager::TraCIBuffer queryTraCIOptional(uint8_t commandId, const TraCIBuffer& buf, bool& success, std::string* errorMsg = 0);

		/**
		 * returns byte-buffer containing a TraCI command with optional parameters
		 */
		std::string makeTraCICommand(uint8_t commandId, TraCIBuffer buf = TraCIBuffer());

		/**
		 * sends a message via TraCI (after adding the header)
		 */
		void sendTraCIMessage(std::string buf);

		/**
		 * receives a message via TraCI (and strips the header)
		 */
		std::string receiveTraCIMessage();

		/**
		 * commonly employed technique to get string values via TraCI
		 */
		std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		Coord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<Coord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);

	public:

		/**
		 * convert TraCI coordinates to OMNeT++ coordinates
		 */
		Coord traci2omnet(TraCICoord coord) const;

		/**
		 * convert OMNeT++ coordinates to TraCI coordinates
		 */
		TraCICoord omnet2traci(Coord coord) const;

	protected:

		/**
		 * convert TraCI angle to OMNeT++ angle (in rad)
		 */
		double traci2omnetAngle(double angle) const;

		/**
		 * convert OMNeT++ angle (in rad) to TraCI angle
		 */
		double omnet2traciAngle(double angle) const;

		void subscribeToVehicleVariables(std::string vehicleId);
		void unsubscribeFromVehicleVariables(std::string vehicleId);
		void processSimSubscription(std::string objectId, TraCIBuffer& buf);
		void processVehicleSubscription(std::string objectId, TraCIBuffer& buf);
		void processSubcriptionResult(TraCIBuffer& buf);
};

template<> void TraCIScenarioManager::TraCIBuffer::write(std::string inv);
template<> void TraCIScenarioManager::TraCIBuffer::write(TraCIScenarioManager::TraCICoord inv);
template<> std::string TraCIScenarioManager::TraCIBuffer::read();
template<> TraCIScenarioManager::TraCICoord TraCIScenarioManager::TraCIBuffer::read();

class TraCIScenarioManagerAccess
{
	public:
		TraCIScenarioManager* get() {
			return FindModule<TraCIScenarioManager*>::findGlobalModule();
		};
};

#endif
