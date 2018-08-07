#include <stdlib.h>

#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/ParBuffer.h"

#include "veins/modules/application/platooning/CC_Const.h"
#define LCA_OVERLAPPING 1 << 13

#ifdef _WIN32
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#endif /* _WIN32 */

namespace Veins {

TraCICommandInterface::TraCICommandInterface(TraCIConnection& c)
    : connection(c)
{
}

std::pair<uint32_t, std::string> TraCICommandInterface::getVersion()
{
    bool success = false;
    TraCIBuffer buf = connection.queryOptional(CMD_GETVERSION, TraCIBuffer(), success);

    if (!success) {
        ASSERT(buf.eof());
        return std::pair<uint32_t, std::string>(0, "(unknown)");
    }

    uint8_t cmdLength;
    buf >> cmdLength;
    uint8_t commandResp;
    buf >> commandResp;
    ASSERT(commandResp == CMD_GETVERSION);
    uint32_t apiVersion;
    buf >> apiVersion;
    std::string serverVersion;
    buf >> serverVersion;
    ASSERT(buf.eof());

    return std::pair<uint32_t, std::string>(apiVersion, serverVersion);
}

void TraCICommandInterface::Vehicle::setSpeedMode(int32_t bitset)
{
    uint8_t variableId = VAR_SPEEDSETMODE;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setSpeed(double speed)
{
    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setMaxSpeed(double speed)
{
    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setColor(const TraCIColor& color)
{
    TraCIBuffer p;
    p << static_cast<uint8_t>(VAR_COLOR);
    p << nodeId;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, p);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::slowDown(double speed, int32_t time)
{
    uint8_t variableId = CMD_SLOWDOWN;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 2;
    uint8_t speedType = TYPE_DOUBLE;
    uint8_t durationType = TYPE_INTEGER;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << speedType << speed << durationType << time);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::newRoute(std::string roadId)
{
    uint8_t variableId = LANE_EDGE_ID;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << roadId);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setParking()
{
    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    uint8_t value = REMOVE_PARKING;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}

std::list<std::string> TraCICommandInterface::getVehicleTypeIds()
{
    return genericGetStringList(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::getRouteIds()
{
    return genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", ID_LIST, RESPONSE_GET_ROUTE_VARIABLE);
}

double TraCICommandInterface::Road::getCurrentTravelTime()
{
    return traci->genericGetDouble(CMD_GET_EDGE_VARIABLE, roadId, VAR_CURRENT_TRAVELTIME, RESPONSE_GET_EDGE_VARIABLE);
}

double TraCICommandInterface::Road::getMeanSpeed()
{
    return traci->genericGetDouble(CMD_GET_EDGE_VARIABLE, roadId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_EDGE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getRoadId()
{
    return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getCurrentRoadOnRoute()
{
    return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, LANE_EDGE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getLaneId()
{
    return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

int32_t TraCICommandInterface::Vehicle::getLaneIndex()
{
    return traci->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getTypeId()
{
    return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getMaxSpeed()
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MAXSPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getLanePosition()
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::Vehicle::getPlannedRoadIds()
{
    return traci->genericGetStringList(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getRouteId()
{
    return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROUTE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::Route::getRoadIds()
{
    return traci->genericGetStringList(CMD_GET_ROUTE_VARIABLE, routeId, VAR_EDGES, RESPONSE_GET_ROUTE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::getRoadIds()
{
    return genericGetStringList(CMD_GET_EDGE_VARIABLE, "", ID_LIST, RESPONSE_GET_EDGE_VARIABLE);
}

void TraCICommandInterface::Vehicle::changeRoute(std::string roadId, double travelTime)
{
    if (travelTime >= 0) {
        uint8_t variableId = VAR_EDGE_TRAVELTIME;
        uint8_t variableType = TYPE_COMPOUND;
        int32_t count = 2;
        uint8_t edgeIdT = TYPE_STRING;
        std::string edgeId = roadId;
        uint8_t newTimeT = TYPE_DOUBLE;
        double newTime = travelTime;
        TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId << newTimeT << newTime);
        ASSERT(buf.eof());
    }
    else {
        uint8_t variableId = VAR_EDGE_TRAVELTIME;
        uint8_t variableType = TYPE_COMPOUND;
        int32_t count = 1;
        uint8_t edgeIdT = TYPE_STRING;
        std::string edgeId = roadId;
        TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId);
        ASSERT(buf.eof());
    }
    {
        uint8_t variableId = CMD_REROUTE_TRAVELTIME;
        uint8_t variableType = TYPE_COMPOUND;
        int32_t count = 0;
        TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count);
        ASSERT(buf.eof());
    }
}

double TraCICommandInterface::Vehicle::getLength()
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getCO2Emissions() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_CO2EMISSION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getCOEmissions() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_COEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getHCEmissions() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_HCEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getPMxEmissions() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_PMXEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getNOxEmissions() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_NOXEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getFuelConsumption() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_FUELCONSUMPTION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getNoiseEmission() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_NOISEEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getElectricityConsumption() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ELECTRICITYCONSUMPTION, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getWaitingTime() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_WAITING_TIME, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getAccumulatedWaitingTime() const
{
    return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_WAITING_TIME_ACCUMULATED, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::getDistance(const Coord& p1, const Coord& p2, bool returnDrivingDistance)
{
    uint8_t variable = DISTANCE_REQUEST;
    std::string simId = "sim0";
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 3;
    uint8_t dType = static_cast<uint8_t>(returnDrivingDistance ? REQUEST_DRIVINGDIST : REQUEST_AIRDIST);

    // query road network boundaries
    TraCIBuffer buf = connection.query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << variable << simId << variableType << count << connection.omnet2traci(p1) << connection.omnet2traci(p2) << dType);
    uint8_t cmdLength_resp;
    buf >> cmdLength_resp;
    uint8_t commandId_resp;
    buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp;
    buf >> variableId_resp;
    ASSERT(variableId_resp == variable);
    std::string simId_resp;
    buf >> simId_resp;
    ASSERT(simId_resp == simId);
    uint8_t typeId_resp;
    buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_DOUBLE);
    double distance;
    buf >> distance;

    ASSERT(buf.eof());

    return distance;
}

void TraCICommandInterface::Vehicle::stopAt(std::string roadId, double pos, uint8_t laneid, double radius, double waittime)
{
    uint8_t variableId = CMD_STOP;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 4;
    uint8_t edgeIdT = TYPE_STRING;
    std::string edgeId = roadId;
    uint8_t stopPosT = TYPE_DOUBLE;
    double stopPos = pos;
    uint8_t stopLaneT = TYPE_BYTE;
    uint8_t stopLane = laneid;
    uint8_t durationT = TYPE_INTEGER;
    uint32_t duration = waittime * 1000;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId << stopPosT << stopPos << stopLaneT << stopLane << durationT << duration);
    ASSERT(buf.eof());
}

std::list<std::string> TraCICommandInterface::getTrafficlightIds()
{
    return genericGetStringList(CMD_GET_TL_VARIABLE, "", ID_LIST, RESPONSE_GET_TL_VARIABLE);
}

std::string TraCICommandInterface::Trafficlight::getCurrentState() const
{
    return traci->genericGetString(CMD_GET_TL_VARIABLE, trafficLightId, TL_RED_YELLOW_GREEN_STATE, RESPONSE_GET_TL_VARIABLE);
}

int32_t TraCICommandInterface::Trafficlight::getDefaultCurrentPhaseDuration() const
{
    return traci->genericGetInt(CMD_GET_TL_VARIABLE, trafficLightId, TL_PHASE_DURATION, RESPONSE_GET_TL_VARIABLE);
}

std::list<std::string> TraCICommandInterface::Trafficlight::getControlledLanes() const
{
    return traci->genericGetStringList(CMD_GET_TL_VARIABLE, trafficLightId, TL_CONTROLLED_LANES, RESPONSE_GET_TL_VARIABLE);
}

std::list<std::list<TraCITrafficLightLink>> TraCICommandInterface::Trafficlight::getControlledLinks() const
{
    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t commandId = CMD_GET_TL_VARIABLE;
    uint8_t variableId = TL_CONTROLLED_LINKS;
    std::string objectId = trafficLightId;
    uint8_t responseId = RESPONSE_GET_TL_VARIABLE;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);
    // generic header
    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);

    int32_t compoundSize;
    buf >> compoundSize; // nr of fields in the compound
    int32_t nrOfSignals = buf.readTypeChecked<int32_t>(TYPE_INTEGER);
    std::list<std::list<TraCITrafficLightLink>> controlledLinks;
    for (int32_t i = 0; i < nrOfSignals; ++i) {
        int32_t nrOfLinks = buf.readTypeChecked<int32_t>(TYPE_INTEGER);
        std::list<TraCITrafficLightLink> linksOfSignal;
        uint8_t stringListType;
        buf >> stringListType;
        ASSERT(stringListType == static_cast<uint8_t>(TYPE_STRINGLIST));
        for (int32_t j = 0; j < nrOfLinks; ++j) {
            int32_t stringCount;
            buf >> stringCount;
            ASSERT(stringCount == 3);
            TraCITrafficLightLink link;
            buf >> link.incoming;
            buf >> link.outgoing;
            buf >> link.internal;
            linksOfSignal.push_back(link);
        }
        controlledLinks.push_back(linksOfSignal);
    }
    return controlledLinks;
}

int32_t TraCICommandInterface::Trafficlight::getCurrentPhaseIndex() const
{
    return traci->genericGetInt(CMD_GET_TL_VARIABLE, trafficLightId, TL_CURRENT_PHASE, RESPONSE_GET_TL_VARIABLE);
}

std::string TraCICommandInterface::Trafficlight::getCurrentProgramID() const
{
    return traci->genericGetString(CMD_GET_TL_VARIABLE, trafficLightId, TL_CURRENT_PROGRAM, RESPONSE_GET_TL_VARIABLE);
}

TraCITrafficLightProgram TraCICommandInterface::Trafficlight::getProgramDefinition() const
{
    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t commandId = CMD_GET_TL_VARIABLE;
    uint8_t variableId = TL_COMPLETE_DEFINITION_RYG;
    std::string objectId = trafficLightId;
    uint8_t responseId = RESPONSE_GET_TL_VARIABLE;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    // generic header
    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);

    int32_t compoundSize;
    buf >> compoundSize; // nr of fields in the compound
    int32_t nrOfLogics = buf.readTypeChecked<int32_t>(TYPE_INTEGER); // nr of logics in the compound

    TraCITrafficLightProgram program(trafficLightId);
    for (int32_t i = 0; i < nrOfLogics; ++i) {
        TraCITrafficLightProgram::Logic logic;
        logic.id = buf.readTypeChecked<std::string>(TYPE_STRING); // program ID
        logic.type = buf.readTypeChecked<int32_t>(TYPE_INTEGER); // (sub)type - currently just a 0
        logic.parameter = buf.readTypeChecked<int32_t>(TYPE_COMPOUND); // (sub)parameter - currently just a 0
        logic.currentPhase = buf.readTypeChecked<int32_t>(TYPE_INTEGER); // phase index

        int32_t nrOfPhases = buf.readTypeChecked<int32_t>(TYPE_INTEGER); // number of phases in this program
        for (int32_t j = 0; j < nrOfPhases; ++j) {
            TraCITrafficLightProgram::Phase phase;
            phase.duration = buf.readTypeChecked<int32_t>(TYPE_INTEGER); // default duration of phase
            phase.minDuration = buf.readTypeChecked<int32_t>(TYPE_INTEGER); // minimum duration of phase
            phase.maxDuration = buf.readTypeChecked<int32_t>(TYPE_INTEGER); // maximum duration of phase
            phase.state = buf.readTypeChecked<std::string>(TYPE_STRING); // phase definition (like "[ryg]*")
            logic.phases.push_back(phase);
        }
        program.addLogic(logic);
    }
    return program;
}

int32_t TraCICommandInterface::Trafficlight::getAssumedNextSwitchTime() const
{
    return traci->genericGetInt(CMD_GET_TL_VARIABLE, trafficLightId, TL_NEXT_SWITCH, RESPONSE_GET_TL_VARIABLE);
}

void TraCICommandInterface::Trafficlight::setState(std::string state)
{
    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_RED_YELLOW_GREEN_STATE) << trafficLightId << static_cast<uint8_t>(TYPE_STRING) << state);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Trafficlight::setPhaseDuration(int32_t duration)
{
    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PHASE_DURATION) << trafficLightId << static_cast<uint8_t>(TYPE_INTEGER) << duration);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Trafficlight::setProgramDefinition(TraCITrafficLightProgram::Logic logic, int32_t logicNr)
{
    TraCIBuffer inbuf;
    inbuf << static_cast<uint8_t>(TL_COMPLETE_PROGRAM_RYG);
    inbuf << trafficLightId;
    inbuf << static_cast<uint8_t>(TYPE_COMPOUND);
    inbuf << logicNr;
    inbuf << static_cast<uint8_t>(TYPE_STRING);
    inbuf << logic.id;
    inbuf << static_cast<uint8_t>(TYPE_INTEGER); // (sub)type - currently unused
    inbuf << logic.type;
    inbuf << static_cast<uint8_t>(TYPE_COMPOUND); // (sub)parameter - currently unused
    inbuf << logic.parameter;
    inbuf << static_cast<uint8_t>(TYPE_INTEGER);
    inbuf << logic.currentPhase;
    inbuf << static_cast<uint8_t>(TYPE_INTEGER);
    inbuf << static_cast<int32_t>(logic.phases.size());

    for (uint32_t i = 0; i < logic.phases.size(); ++i) {
        TraCITrafficLightProgram::Phase& phase = logic.phases[i];
        inbuf << static_cast<uint8_t>(TYPE_INTEGER);
        inbuf << phase.duration;
        inbuf << static_cast<uint8_t>(TYPE_INTEGER);
        inbuf << phase.minDuration;
        inbuf << static_cast<uint8_t>(TYPE_INTEGER);
        inbuf << phase.maxDuration;
        inbuf << static_cast<uint8_t>(TYPE_STRING);
        inbuf << phase.state;
    }
    TraCIBuffer obuf = connection->query(CMD_SET_TL_VARIABLE, inbuf);
    ASSERT(obuf.eof());
}

void TraCICommandInterface::Trafficlight::setProgram(std::string program)
{
    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PROGRAM) << trafficLightId << static_cast<uint8_t>(TYPE_STRING) << program);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Trafficlight::setPhaseIndex(int32_t index)
{
    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PHASE_INDEX) << trafficLightId << static_cast<uint8_t>(TYPE_INTEGER) << index);
    ASSERT(buf.eof());
}

std::list<std::string> TraCICommandInterface::getPolygonIds()
{
    return genericGetStringList(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);
}

std::string TraCICommandInterface::Polygon::getTypeId()
{
    return traci->genericGetString(CMD_GET_POLYGON_VARIABLE, polyId, VAR_TYPE, RESPONSE_GET_POLYGON_VARIABLE);
}

std::list<Coord> TraCICommandInterface::Polygon::getShape()
{
    return traci->genericGetCoordList(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);
}

void TraCICommandInterface::Polygon::setShape(const std::list<Coord>& points)
{
    TraCIBuffer buf;
    uint8_t count = static_cast<uint8_t>(points.size());
    buf << static_cast<uint8_t>(VAR_SHAPE) << polyId << static_cast<uint8_t>(TYPE_POLYGON) << count;
    for (std::list<Coord>::const_iterator i = points.begin(); i != points.end(); ++i) {
        const TraCICoord& pos = connection->omnet2traci(*i);
        buf << static_cast<double>(pos.x) << static_cast<double>(pos.y);
    }
    TraCIBuffer obuf = connection->query(CMD_SET_POLYGON_VARIABLE, buf);
    ASSERT(obuf.eof());
}

void TraCICommandInterface::addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points)
{
    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << polyId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
    p << static_cast<uint8_t>(TYPE_STRING) << polyType;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    p << static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(filled);
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << static_cast<uint8_t>(TYPE_POLYGON) << static_cast<uint8_t>(points.size());
    for (std::list<Coord>::const_iterator i = points.begin(); i != points.end(); ++i) {
        const TraCICoord& pos = connection.omnet2traci(*i);
        p << static_cast<double>(pos.x) << static_cast<double>(pos.y);
    }

    TraCIBuffer buf = connection.query(CMD_SET_POLYGON_VARIABLE, p);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Polygon::remove(int32_t layer)
{
    TraCIBuffer p;

    p << static_cast<uint8_t>(REMOVE) << polyId;
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;

    TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, p);
    ASSERT(buf.eof());
}

void TraCICommandInterface::addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos_)
{
    TraCIBuffer p;

    TraCICoord pos = connection.omnet2traci(pos_);
    p << static_cast<uint8_t>(ADD) << poiId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(4);
    p << static_cast<uint8_t>(TYPE_STRING) << poiType;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << pos;

    TraCIBuffer buf = connection.query(CMD_SET_POI_VARIABLE, p);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Poi::remove(int32_t layer)
{
    TraCIBuffer p;

    p << static_cast<uint8_t>(REMOVE) << poiId;
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;

    TraCIBuffer buf = connection->query(CMD_SET_POI_VARIABLE, p);
    ASSERT(buf.eof());
}

std::list<std::string> TraCICommandInterface::getLaneIds()
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}

std::list<Coord> TraCICommandInterface::Lane::getShape()
{
    return traci->genericGetCoordList(CMD_GET_LANE_VARIABLE, laneId, VAR_SHAPE, RESPONSE_GET_LANE_VARIABLE);
}

std::string TraCICommandInterface::Lane::getRoadId()
{
    return traci->genericGetString(CMD_GET_LANE_VARIABLE, laneId, LANE_EDGE_ID, RESPONSE_GET_LANE_VARIABLE);
}

double TraCICommandInterface::Lane::getLength()
{
    return traci->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}

double TraCICommandInterface::Lane::getMaxSpeed()
{
    return traci->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_MAXSPEED, RESPONSE_GET_LANE_VARIABLE);
}

double TraCICommandInterface::Lane::getMeanSpeed()
{
    return traci->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::getJunctionIds()
{
    return genericGetStringList(CMD_GET_JUNCTION_VARIABLE, "", ID_LIST, RESPONSE_GET_JUNCTION_VARIABLE);
}

Coord TraCICommandInterface::Junction::getPosition()
{
    return traci->genericGetCoord(CMD_GET_JUNCTION_VARIABLE, junctionId, VAR_POSITION, RESPONSE_GET_JUNCTION_VARIABLE);
}

bool TraCICommandInterface::addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st, double emitPosition, double emitSpeed, int8_t emitLane)
{
    bool success = false;

    uint8_t variableId = ADD;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 6;
    int32_t emitTime = (emitTime_st < 0) ? round(emitTime_st.dbl()) : (floor(emitTime_st.dbl() * 1000));
    TraCIBuffer buf = connection.queryOptional(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << vehicleId << variableType << count << (uint8_t) TYPE_STRING << vehicleTypeId << (uint8_t) TYPE_STRING << routeId << (uint8_t) TYPE_INTEGER << emitTime << (uint8_t) TYPE_DOUBLE << emitPosition << (uint8_t) TYPE_DOUBLE << emitSpeed << (uint8_t) TYPE_BYTE << emitLane, success);
    ASSERT(buf.eof());

    return success;
}

bool TraCICommandInterface::Vehicle::changeVehicleRoute(const std::list<std::string>& edges)
{
    if (getRoadId().find(':') != std::string::npos) return false;
    if (edges.front().compare(getRoadId()) != 0) return false;
    uint8_t variableId = VAR_ROUTE;
    uint8_t variableType = TYPE_STRINGLIST;
    TraCIBuffer buf;
    buf << variableId << nodeId << variableType;
    int32_t numElem = edges.size();
    buf << numElem;
    for (std::list<std::string>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
        buf << static_cast<std::string>(*i);
    }
    TraCIBuffer obuf = connection->query(CMD_SET_VEHICLE_VARIABLE, buf);
    ASSERT(obuf.eof());
    return true;
}

void TraCICommandInterface::Vehicle::setLaneChangeMode(int mode)
{
    uint8_t variableId = VAR_LANECHANGE_MODE;
    uint8_t type = TYPE_INTEGER;
    TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << type << mode);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::getLaneChangeState(int direction, int& state1, int& state2)
{
    TraCIBuffer response = traci->connection.query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(CMD_CHANGELANE) << nodeId << static_cast<uint8_t>(TYPE_INTEGER) << direction);
    uint8_t cmdLength;
    response >> cmdLength;
    uint8_t responseId;
    response >> responseId;
    ASSERT(responseId == RESPONSE_GET_VEHICLE_VARIABLE);
    uint8_t variable;
    response >> variable;
    ASSERT(variable == CMD_CHANGELANE);
    std::string id;
    response >> id;
    uint8_t type;
    response >> type;
    ASSERT(type == TYPE_COMPOUND);
    int count;
    response >> count;
    ASSERT(count == 2);
    response >> type;
    ASSERT(type == TYPE_INTEGER);
    response >> state1;
    response >> type;
    ASSERT(type == TYPE_INTEGER);
    response >> state2;
}

void TraCICommandInterface::Vehicle::changeLane(int lane, int duration)
{
    uint8_t commandType = TYPE_COMPOUND;
    int nParameters = 2;
    uint8_t variableId = CMD_CHANGELANE;
    TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << commandType << nParameters << static_cast<uint8_t>(TYPE_BYTE) << (uint8_t) lane << static_cast<uint8_t>(TYPE_INTEGER) << duration);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setParameter(const std::string& parameter, int value)
{
    std::stringstream strValue;
    strValue << value;
    setParameter(parameter, strValue.str());
}

void TraCICommandInterface::Vehicle::setParameter(const std::string& parameter, double value)
{
    std::stringstream strValue;
    strValue << value;
    setParameter(parameter, strValue.str());
}

void TraCICommandInterface::Vehicle::setParameter(const std::string& parameter, const std::string& value)
{
    static int32_t nParameters = 2;
    std::stringstream par;
    par << "carFollowModel." << parameter;
    TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_PARAMETER) << nodeId << static_cast<uint8_t>(TYPE_COMPOUND) << nParameters << static_cast<uint8_t>(TYPE_STRING) << par.str() << static_cast<uint8_t>(TYPE_STRING) << value);
    ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::getParameter(const std::string& parameter, int& value)
{
    std::string v;
    getParameter(parameter, v);
    ParBuffer buf(v);
    buf >> value;
}
void TraCICommandInterface::Vehicle::getParameter(const std::string& parameter, double& value)
{
    std::string v;
    getParameter(parameter, v);
    ParBuffer buf(v);
    buf >> value;
}

void TraCICommandInterface::Vehicle::getParameter(const std::string& parameter, std::string& value)
{
    std::stringstream par;
    par << "carFollowModel." << parameter;
    TraCIBuffer response = traci->connection.query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_PARAMETER) << nodeId << static_cast<uint8_t>(TYPE_STRING) << par.str());
    uint8_t cmdLength;
    response >> cmdLength;
    uint8_t responseId;
    response >> responseId;
    ASSERT(responseId == RESPONSE_GET_VEHICLE_VARIABLE);
    uint8_t variable;
    response >> variable;
    ASSERT(variable == VAR_PARAMETER);
    std::string id;
    response >> id;
    ASSERT(id == nodeId);
    uint8_t type;
    response >> type;
    ASSERT(type == TYPE_STRING);
    response >> value;
}

std::pair<double, double> TraCICommandInterface::getLonLat(const Coord& coord)
{
    TraCIBuffer request;
    request << static_cast<uint8_t>(POSITION_CONVERSION) << std::string("sim0") << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(2) << connection.omnet2traci(coord) << static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(POSITION_LON_LAT);
    TraCIBuffer response = connection.query(CMD_GET_SIM_VARIABLE, request);

    uint8_t cmdLength;
    response >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        response >> cmdLengthX;
    }
    uint8_t responseId;
    response >> responseId;
    ASSERT(responseId == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variable;
    response >> variable;
    ASSERT(variable == POSITION_CONVERSION);
    std::string id;
    response >> id;
    uint8_t convPosType;
    response >> convPosType;
    ASSERT(convPosType == POSITION_LON_LAT);
    double convPosLon;
    response >> convPosLon;
    double convPosLat;
    response >> convPosLat;

    return std::make_pair(convPosLon, convPosLat);
}

void TraCICommandInterface::GuiView::setScheme(std::string name)
{
    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_VIEW_SCHEMA) << viewId << static_cast<uint8_t>(TYPE_STRING) << name);
    ASSERT(buf.eof());
}

void TraCICommandInterface::GuiView::setZoom(double zoom)
{
    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_VIEW_ZOOM) << viewId << static_cast<uint8_t>(TYPE_DOUBLE) << zoom);
    ASSERT(buf.eof());
}

void TraCICommandInterface::GuiView::setBoundary(Coord p1_, Coord p2_)
{
    TraCICoord p1 = connection->omnet2traci(p1_);
    TraCICoord p2 = connection->omnet2traci(p2_);

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_VIEW_BOUNDARY) << viewId << static_cast<uint8_t>(TYPE_BOUNDINGBOX) << p1.x << p1.y << p2.x << p2.y);
    ASSERT(buf.eof());
}

void TraCICommandInterface::GuiView::takeScreenshot(std::string filename)
{
    if (filename == "") {
        // get absolute path of results/ directory
        const char* myResultsDir = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RESULTDIR);
        char* s = realpath(myResultsDir, 0);
        std::string absolutePath = s;
        free(s);

        // get run id
        const char* myRunID = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNID);

        // build filename from this
        char ss[512];
        snprintf(ss, sizeof(ss), "%s/screenshot-%s-@%08.2f.png", absolutePath.c_str(), myRunID, simTime().dbl());
        filename = ss;
    }

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_SCREENSHOT) << viewId << static_cast<uint8_t>(TYPE_STRING) << filename);
    ASSERT(buf.eof());
}

void TraCICommandInterface::GuiView::trackVehicle(std::string vehicleId)
{
    TraCIScenarioManager* manager = FindModule<TraCIScenarioManager*>::findGlobalModule();
    // don't send gui commands when gui is not active or sumo will crash
    if (manager->isGuiSimulation()) {
        TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_TRACK_VEHICLE) << viewId << static_cast<uint8_t>(TYPE_STRING) << vehicleId);
        ASSERT(buf.eof());
    }
}

std::string TraCICommandInterface::genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_STRING;
    std::string res;

    TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}

Coord TraCICommandInterface::genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{

    uint8_t resultTypeId = POSITION_2D;
    double x;
    double y;

    TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> x;
    buf >> y;

    ASSERT(buf.eof());

    return connection.traci2omnet(TraCICoord(x, y));
}

double TraCICommandInterface::genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{

    uint8_t resultTypeId = TYPE_DOUBLE;
    double res;

    TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}

int32_t TraCICommandInterface::genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{

    uint8_t resultTypeId = TYPE_INTEGER;
    int32_t res;

    TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}

std::list<std::string> TraCICommandInterface::genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{

    uint8_t resultTypeId = TYPE_STRINGLIST;
    std::list<std::string> res;

    TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count;
    buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        std::string id;
        buf >> id;
        res.push_back(id);
    }

    ASSERT(buf.eof());

    return res;
}

std::list<Coord> TraCICommandInterface::genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{

    uint8_t resultTypeId = TYPE_POLYGON;
    std::list<Coord> res;

    TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint8_t count;
    buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        double x;
        buf >> x;
        double y;
        buf >> y;
        res.push_back(connection.traci2omnet(TraCICoord(x, y)));
    }

    ASSERT(buf.eof());

    return res;
}

void TraCICommandInterface::Vehicle::getVehicleData(double& speed, double& acceleration, double& controllerAcceleration, double& positionX, double& positionY, double& time)
{
    std::string v;
    getParameter(PAR_SPEED_AND_ACCELERATION, v);
    ParBuffer buf(v);
    buf >> speed >> acceleration >> controllerAcceleration >> positionX >> positionY >> time;
}

void TraCICommandInterface::Vehicle::getVehicleData(Plexe::VEHICLE_DATA* data)
{
    std::string v;
    getParameter(PAR_SPEED_AND_ACCELERATION, v);
    ParBuffer buf(v);
    buf >> data->speed >> data->acceleration >> data->u >> data->positionX >> data->positionY >> data->time >> data->speedX >> data->speedY >> data->angle;
}

double TraCICommandInterface::Vehicle::getACCAcceleration()
{
    double v;
    getParameter(PAR_ACC_ACCELERATION, v);
    return v;
}

void TraCICommandInterface::Vehicle::setLeaderVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time)
{
    ParBuffer buf;
    buf << speed << acceleration << positionX << positionY << time << controllerAcceleration;
    setParameter(PAR_LEADER_SPEED_AND_ACCELERATION, buf.str());
}

void TraCICommandInterface::Vehicle::setPlatoonLeaderData(double speed, double acceleration, double positionX, double positionY, double time)
{
    std::cout << "setPlatoonLeaderData() is deprecated and will be removed. Please use setLeaderVehicleData()\n";
    setLeaderVehicleData(acceleration, acceleration, speed, positionX, positionY, time);
}

void TraCICommandInterface::Vehicle::setFrontVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time)
{
    ParBuffer buf;
    buf << speed << acceleration << positionX << positionY << time << controllerAcceleration;
    setParameter(PAR_PRECEDING_SPEED_AND_ACCELERATION, buf.str());
}

void TraCICommandInterface::Vehicle::setPrecedingVehicleData(double speed, double acceleration, double positionX, double positionY, double time)
{
    std::cout << "setPrecedingVehicleData() is deprecated and will be removed. Please use setFrontVehicleData()\n";
    setFrontVehicleData(acceleration, acceleration, speed, positionX, positionY, time);
}

unsigned int TraCICommandInterface::Vehicle::getLanesCount()
{
    int v;
    getParameter(PAR_LANES_COUNT, v);
    return (unsigned int) v;
}

void TraCICommandInterface::Vehicle::setCruiseControlDesiredSpeed(double desiredSpeed)
{
    setParameter(PAR_CC_DESIRED_SPEED, desiredSpeed);
}

const double TraCICommandInterface::Vehicle::getCruiseControlDesiredSpeed()
{
    double desiredSpeed;
    getParameter(PAR_CC_DESIRED_SPEED, desiredSpeed);
    return desiredSpeed;
}

void TraCICommandInterface::Vehicle::setActiveController(int activeController)
{
    setParameter(PAR_ACTIVE_CONTROLLER, activeController);
}

int TraCICommandInterface::Vehicle::getActiveController()
{
    int v;
    getParameter(PAR_ACTIVE_CONTROLLER, v);
    return v;
}

void TraCICommandInterface::Vehicle::setCACCConstantSpacing(double spacing)
{
    setParameter(PAR_CACC_SPACING, spacing);
}
double TraCICommandInterface::Vehicle::getCACCConstantSpacing()
{
    double v;
    getParameter(PAR_CACC_SPACING, v);
    return v;
}

void TraCICommandInterface::Vehicle::setPathCACCParameters(double omegaN, double xi, double c1, double distance)
{
    if (omegaN >= 0) setParameter(CC_PAR_CACC_OMEGA_N, omegaN);
    if (xi >= 0) setParameter(CC_PAR_CACC_XI, xi);
    if (c1 >= 0) setParameter(CC_PAR_CACC_C1, c1);
    if (distance >= 0) setParameter(PAR_CACC_SPACING, distance);
}

void TraCICommandInterface::Vehicle::setPloegCACCParameters(double kp, double kd, double h)
{
    if (kp >= 0) setParameter(CC_PAR_PLOEG_KP, kp);
    if (kd >= 0) setParameter(CC_PAR_PLOEG_KD, kd);
    if (h >= 0) setParameter(CC_PAR_PLOEG_H, h);
}

void TraCICommandInterface::Vehicle::setACCHeadwayTime(double headway)
{
    setParameter(PAR_ACC_HEADWAY_TIME, headway);
}

double TraCICommandInterface::Vehicle::getACCHeadwayTime()
{
    double headway;
    getParameter(PAR_ACC_HEADWAY_TIME, headway);
    return headway;
}

void TraCICommandInterface::Vehicle::setFixedAcceleration(int activate, double acceleration)
{
    ParBuffer buf;
    buf << activate << acceleration;
    setParameter(PAR_FIXED_ACCELERATION, buf.str());
}

bool TraCICommandInterface::Vehicle::isCrashed()
{
    int crashed;
    getParameter(PAR_CRASHED, crashed);
    return crashed;
}

void TraCICommandInterface::Vehicle::setLaneChangeAction(int action)
{

    std::cout << "setLaneChangeAction API is deprecated. Please remove it from your code\n";
    uint8_t variableId = VAR_LANECHANGE_MODE;
    uint8_t type = TYPE_INTEGER;
    int traciAction;
    if (action == Plexe::MOVE_TO_FIXED_LANE || action == Plexe::STAY_IN_CURRENT_LANE)
        traciAction = FIX_LC;
    else
        traciAction = DEFAULT_NOTRACI_LC;
    TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << type << traciAction);
    ASSERT(buf.eof());
}

void TraCICommandInterface::executePlexeTimestep()
{
    std::vector<PlexeLaneChanges::iterator> satisfied;
    for (auto i = laneChanges.begin(); i != laneChanges.end(); i++) {
        if (i->second.wait) {
            i->second.wait = false;
            continue;
        }
        int current = vehicle(i->first).getLaneIndex();
        int nLanes = i->second.lane - current;
        int direction;
        if (nLanes > 0)
            direction = 1;
        else if (nLanes < 0)
            direction = -1;
        else
            direction = 0;

        if (direction == 0) {
            satisfied.push_back(i);
            if (i->second.safe)
                vehicle(i->first).setLaneChangeMode(FIX_LC);
            else
                vehicle(i->first).setLaneChangeMode(FIX_LC_AGGRESSIVE);
        }
        else {
            __changeLane(i->first, current, direction, i->second.safe);
        }
    }
    for (int i = 0; i < satisfied.size(); i++) laneChanges.erase(satisfied[i]);
}

void TraCICommandInterface::__changeLane(std::string veh, int current, int direction, bool safe)
{
    if (safe) {
        vehicle(veh).setLaneChangeMode(FIX_LC);
        vehicle(veh).changeLane(current + direction, 0);
    }
    else {
        int state, state2;
        vehicle(veh).getLaneChangeState(direction, state, state2);
        if ((state & LCA_OVERLAPPING) == 0) {
            vehicle(veh).setLaneChangeMode(FIX_LC_AGGRESSIVE);
            vehicle(veh).changeLane(current + direction, 0);
            laneChanges[veh].wait = true;
        }
    }
}

void TraCICommandInterface::Vehicle::setFixedLane(int8_t laneIndex, bool safe)
{

    if (laneIndex == -1) {
        setLaneChangeMode(DEFAULT_NOTRACI_LC);
        return;
    }

    PlexeLaneChange lc;
    lc.lane = laneIndex;
    lc.safe = safe;
    lc.wait = false;
    traci->laneChanges[nodeId] = lc;
}

void TraCICommandInterface::Vehicle::getRadarMeasurements(double& distance, double& relativeSpeed)
{
    std::string v;
    getParameter(PAR_RADAR_DATA, v);
    ParBuffer buf(v);
    buf >> distance >> relativeSpeed;
}

void TraCICommandInterface::Vehicle::setLeaderVehicleFakeData(double controllerAcceleration, double acceleration, double speed)
{
    ParBuffer buf;
    buf << speed << acceleration << controllerAcceleration;
    setParameter(PAR_LEADER_FAKE_DATA, buf.str());
}

void TraCICommandInterface::Vehicle::setLeaderFakeData(double leaderSpeed, double leaderAcceleration)
{
    std::cout << "setLeaderFakeData() is deprecated and will be removed. Please use setLeaderVehicleFakeData()\n";
    setLeaderVehicleFakeData(leaderAcceleration, leaderAcceleration, leaderSpeed);
}

void TraCICommandInterface::Vehicle::setFrontVehicleFakeData(double controllerAcceleration, double acceleration, double speed, double distance)
{
    ParBuffer buf;
    buf << speed << acceleration << distance << controllerAcceleration;
    setParameter(PAR_FRONT_FAKE_DATA, buf.str());
}

void TraCICommandInterface::Vehicle::setFrontFakeData(double frontDistance, double frontSpeed, double frontAcceleration)
{
    std::cout << "setFrontFakeData() is deprecated and will be removed. Please use setFrontVehicleFakeData()\n";
    setFrontVehicleFakeData(frontAcceleration, frontAcceleration, frontSpeed, frontDistance);
}

double TraCICommandInterface::Vehicle::getDistanceFromRouteBegin()
{
    double v;
    getParameter(PAR_DISTANCE_FROM_BEGIN, v);
    return v;
}

double TraCICommandInterface::Vehicle::getDistanceToRouteEnd()
{
    double v;
    getParameter(PAR_DISTANCE_TO_END, v);
    return v;
}

std::string TraCICommandInterface::Vehicle::getVType()
{
    return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}

void TraCICommandInterface::Vehicle::setVehicleData(const struct Plexe::VEHICLE_DATA* data)
{
    ParBuffer buf;
    buf << data->index << data->speed << data->acceleration << data->positionX << data->positionY << data->time << data->length << data->u << data->speedX << data->speedY << data->angle;
    setParameter(CC_PAR_VEHICLE_DATA, buf.str());
}

void TraCICommandInterface::Vehicle::getStoredVehicleData(struct Plexe::VEHICLE_DATA* data, int index)
{
    ParBuffer inBuf;
    std::string v;
    inBuf << CC_PAR_VEHICLE_DATA << index;
    getParameter(inBuf.str(), v);
    ParBuffer outBuf(v);
    outBuf >> data->index >> data->speed >> data->acceleration >> data->positionX >> data->positionY >> data->time >> data->length >> data->u >> data->speedX >> data->speedY >> data->angle;
}

void TraCICommandInterface::Vehicle::useControllerAcceleration(bool use)
{
    setParameter(PAR_USE_CONTROLLER_ACCELERATION, use ? 1 : 0);
}

void TraCICommandInterface::Vehicle::getEngineData(int& gear, double& rpm)
{
    ParBuffer inBuf;
    std::string v;
    inBuf << PAR_ENGINE_DATA;
    getParameter(inBuf.str(), v);
    ParBuffer outBuf(v);
    outBuf >> gear >> rpm;
}

void TraCICommandInterface::Vehicle::enableAutoFeed(bool enable, std::string leaderId, std::string frontId)
{
    if (enable && (leaderId.compare("") == 0 || frontId.compare("") == 0)) return;
    ParBuffer inBuf;
    if (enable)
        inBuf << 1 << leaderId << frontId;
    else
        inBuf << 0;
    setParameter(PAR_USE_AUTO_FEEDING, inBuf.str());
}

void TraCICommandInterface::Vehicle::usePrediction(bool enable)
{
    setParameter(PAR_USE_PREDICTION, enable ? 1 : 0);
}

void TraCICommandInterface::Vehicle::addPlatoonMember(std::string memberId, int position)
{
    ParBuffer inBuf;
    inBuf << memberId << position;
    setParameter(PAR_ADD_MEMBER, inBuf.str());
}

void TraCICommandInterface::Vehicle::removePlatoonMember(std::string memberId)
{
    setParameter(PAR_REMOVE_MEMBER, memberId);
}

void TraCICommandInterface::Vehicle::enableAutoLaneChanging(bool enable)
{
    setParameter(PAR_ENABLE_AUTO_LANE_CHANGE, enable ? 1 : 0);
}
} // namespace Veins
