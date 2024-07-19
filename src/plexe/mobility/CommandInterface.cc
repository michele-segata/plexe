//
// Copyright (C) 2018-2023 Michele Segata <segata@ccs-labs.org>
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

#include "CommandInterface.h"

#include <veins/modules/mobility/traci/TraCIScenarioManager.h>
#include <veins/modules/mobility/traci/TraCIConnection.h>
#include <veins/modules/mobility/traci/TraCIConstants.h>
#include <veins/modules/mobility/traci/ParBuffer.h>
#include "plexe/utilities/BasePositionHelper.h"

#include <sstream>

using veins::ParBuffer;
using veins::TraCIBuffer;
using namespace veins::TraCIConstants;

namespace plexe {
namespace traci {

CommandInterface::CommandInterface(cComponent* owner, veins::TraCICommandInterface* veinsCommandInterface, veins::TraCIConnection* connection)
    : HasLogProxy(owner)
    , veinsCommandInterface(veinsCommandInterface)
    , connection(connection)
{
}

void CommandInterface::Vehicle::setLaneChangeMode(int mode)
{
    uint8_t variableId = VAR_LANECHANGE_MODE;
    uint8_t type = TYPE_INTEGER;
    TraCIBuffer buf = cifc->connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << type << mode);
    ASSERT(buf.eof());
}

void CommandInterface::Vehicle::getLaneChangeState(int direction, int& state1, int& state2)
{
    TraCIBuffer response = cifc->connection->query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(CMD_CHANGELANE) << nodeId << static_cast<uint8_t>(TYPE_INTEGER) << direction);
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

void CommandInterface::Vehicle::changeLane(int lane, double duration)
{
    performPlatoonLaneChange(lane);
}

std::vector<CommandInterface::Vehicle::neighbor> CommandInterface::Vehicle::getNeighbors(uint8_t lateralDirection, uint8_t longitudinalDirection, uint8_t blocking)
{
    TraCIBuffer response = cifc->connection->query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_NEIGHBORS)
        << nodeId
        << static_cast<uint8_t>(TYPE_UBYTE)
        << static_cast<uint8_t>(blocking<<2 | longitudinalDirection<<1 | lateralDirection));

    uint8_t cmdLength;
    response >> cmdLength;
    uint8_t responseId;
    response >> responseId;
    ASSERT(responseId == RESPONSE_GET_VEHICLE_VARIABLE);
    uint8_t variable;
    response >> variable;
    ASSERT(variable == VAR_NEIGHBORS);
    std::string id;
    response >> id;
    uint8_t type;
    response >> type;
    ASSERT(type == TYPE_STRINGLIST);
    int len;
    response >> len;

    std::vector<neighbor> neighbors;
    std::string vehicleName;
    double distance;
    for (int i= 0; i < len; i++)
    {
        response >> vehicleName;
        response >> distance;

        neighbors.push_back(neighbor(vehicleName, distance));
    }

    return neighbors;
}

void CommandInterface::Vehicle::setLeaderVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time)
{
    ParBuffer buf;
    buf << speed << acceleration << positionX << positionY << time << controllerAcceleration;
    veinsVehicle().setParameter(PAR_LEADER_SPEED_AND_ACCELERATION, buf.str());
}

void CommandInterface::Vehicle::setPlatoonLeaderData(double speed, double acceleration, double positionX, double positionY, double time)
{
    std::cout << "setPlatoonLeaderData() is deprecated and will be removed. Please use setLeaderVehicleData()\n";
    setLeaderVehicleData(acceleration, acceleration, speed, positionX, positionY, time);
}

void CommandInterface::Vehicle::setFrontVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time)
{
    ParBuffer buf;
    buf << speed << acceleration << positionX << positionY << time << controllerAcceleration;
    veinsVehicle().setParameter(PAR_PRECEDING_SPEED_AND_ACCELERATION, buf.str());
}

void CommandInterface::Vehicle::getVehicleData(double& speed, double& acceleration, double& controllerAcceleration, double& positionX, double& positionY, double& time)
{
    std::string v;
    veinsVehicle().getParameter(PAR_SPEED_AND_ACCELERATION, v);
    ParBuffer buf(v);
    buf >> speed >> acceleration >> controllerAcceleration >> positionX >> positionY >> time;
}

void CommandInterface::Vehicle::getVehicleData(VEHICLE_DATA* data)
{
    std::string v;
    veinsVehicle().getParameter(PAR_SPEED_AND_ACCELERATION, v);
    ParBuffer buf(v);
    buf >> data->speed >> data->acceleration >> data->u >> data->positionX >> data->positionY >> data->time >> data->speedX >> data->speedY >> data->angle;
}

void CommandInterface::Vehicle::setCruiseControlDesiredSpeed(double desiredSpeed)
{
    veinsVehicle().setParameter(PAR_CC_DESIRED_SPEED, desiredSpeed);
}

const double CommandInterface::Vehicle::getCruiseControlDesiredSpeed()
{
    double desiredSpeed;
    veinsVehicle().getParameter(PAR_CC_DESIRED_SPEED, desiredSpeed);
    return desiredSpeed;
}

void CommandInterface::Vehicle::setActiveController(int activeController)
{
    if (activeController != FAKED_CACC)
        veinsVehicle().setParameter(PAR_ACTIVE_CONTROLLER, activeController);
    else
        throw cRuntimeError("Activacting FAKED_CACC must be done through the activateFakedCACC() API to specify a role and a future predecessor");
}

void CommandInterface::Vehicle::activateFakedCACC(int targetController, enum FAKED_CACC_ROLE role, std::string futurePredecessor)
{
    ParBuffer buf;
    buf << targetController << (int)role << futurePredecessor;
    veinsVehicle().setParameter(PAR_ACTIVE_FAKED_CACC, buf.str());
}

int CommandInterface::Vehicle::getActiveController()
{
    int v;
    veinsVehicle().getParameter(PAR_ACTIVE_CONTROLLER, v);
    return v;
}

void CommandInterface::Vehicle::setCACCConstantSpacing(double spacing)
{
    veinsVehicle().setParameter(PAR_CACC_SPACING, spacing);
}

double CommandInterface::Vehicle::getCACCConstantSpacing()
{
    double v;
    veinsVehicle().getParameter(PAR_CACC_SPACING, v);
    return v;
}

void CommandInterface::Vehicle::setPathCACCParameters(double omegaN, double xi, double c1, double distance)
{
    if (omegaN >= 0) veinsVehicle().setParameter(CC_PAR_CACC_OMEGA_N, omegaN);
    if (xi >= 0) veinsVehicle().setParameter(CC_PAR_CACC_XI, xi);
    if (c1 >= 0) veinsVehicle().setParameter(CC_PAR_CACC_C1, c1);
    if (distance >= 0) veinsVehicle().setParameter(PAR_CACC_SPACING, distance);
}

void CommandInterface::Vehicle::setPloegCACCParameters(double kp, double kd, double h)
{
    if (kp >= 0) veinsVehicle().setParameter(CC_PAR_PLOEG_KP, kp);
    if (kd >= 0) veinsVehicle().setParameter(CC_PAR_PLOEG_KD, kd);
    if (h >= 0) veinsVehicle().setParameter(CC_PAR_PLOEG_H, h);
}

void CommandInterface::Vehicle::setACCHeadwayTime(double headway)
{
    veinsVehicle().setParameter(PAR_ACC_HEADWAY_TIME, headway);
}

double CommandInterface::Vehicle::getACCHeadwayTime()
{
    double headway;
    veinsVehicle().getParameter(PAR_ACC_HEADWAY_TIME, headway);
    return headway;
}

void CommandInterface::Vehicle::setFixedAcceleration(int activate, double acceleration)
{
    ParBuffer buf;
    buf << activate << acceleration;
    veinsVehicle().setParameter(PAR_FIXED_ACCELERATION, buf.str());
}

bool CommandInterface::Vehicle::isCrashed()
{
    int crashed;
    veinsVehicle().getParameter(PAR_CRASHED, crashed);
    return crashed;
}

void CommandInterface::Vehicle::setFixedLane(int8_t laneIndex, bool safe)
{

    if (laneIndex == -1) {
        // give back total control to sumo (e.g., when using human driven vehicles)
        setLaneChangeMode(DEFAULT_NOTRACI_LC);
        return;
    }
    else {
        setLaneChangeMode(FIX_LC);
        changeLane(laneIndex, 0);
    }
}

void CommandInterface::Vehicle::getRadarMeasurements(double& distance, double& relativeSpeed)
{
    std::string v;
    veinsVehicle().getParameter(PAR_RADAR_DATA, v);
    ParBuffer buf(v);
    buf >> distance >> relativeSpeed;
}

void CommandInterface::Vehicle::setLeaderVehicleFakeData(double controllerAcceleration, double acceleration, double speed)
{
    ParBuffer buf;
    buf << speed << acceleration << controllerAcceleration;
    veinsVehicle().setParameter(PAR_LEADER_FAKE_DATA, buf.str());
}

void CommandInterface::Vehicle::setLeaderFakeData(double leaderSpeed, double leaderAcceleration)
{
    std::cout << "setLeaderFakeData() is deprecated and will be removed. Please use setLeaderVehicleFakeData()\n";
    setLeaderVehicleFakeData(leaderAcceleration, leaderAcceleration, leaderSpeed);
}

void CommandInterface::Vehicle::setFrontVehicleFakeData(double controllerAcceleration, double acceleration, double speed, double distance)
{
    ParBuffer buf;
    buf << speed << acceleration << distance << controllerAcceleration;
    veinsVehicle().setParameter(PAR_FRONT_FAKE_DATA, buf.str());
}

void CommandInterface::Vehicle::setPrecedingVehicleData(double speed, double acceleration, double positionX, double positionY, double time)
{
    std::cout << "setPrecedingVehicleData() is deprecated and will be removed. Please use setFrontVehicleData()\n";
    setFrontVehicleData(acceleration, acceleration, speed, positionX, positionY, time);
}

void CommandInterface::Vehicle::setFrontFakeData(double frontDistance, double frontSpeed, double frontAcceleration)
{
    std::cout << "setFrontFakeData() is deprecated and will be removed. Please use setFrontVehicleFakeData()\n";
    setFrontVehicleFakeData(frontAcceleration, frontAcceleration, frontSpeed, frontDistance);
}

double CommandInterface::Vehicle::getDistanceToRouteEnd()
{
    double v;
    veinsVehicle().getParameter(PAR_DISTANCE_TO_END, v);
    return v;
}

double CommandInterface::Vehicle::getDistanceFromRouteBegin()
{
    double v;
    veinsVehicle().getParameter(PAR_DISTANCE_FROM_BEGIN, v);
    return v;
}

double CommandInterface::Vehicle::getACCAcceleration()
{
    double v;
    veinsVehicle().getParameter(PAR_ACC_ACCELERATION, v);
    return v;
}

void CommandInterface::Vehicle::setVehicleData(const struct VEHICLE_DATA* data)
{
    ParBuffer buf;
    buf << data->index << data->speed << data->acceleration << data->positionX << data->positionY << data->time << data->length << data->u << data->speedX << data->speedY << data->angle;
    veinsVehicle().setParameter(CC_PAR_VEHICLE_DATA, buf.str());
}

void CommandInterface::Vehicle::getStoredVehicleData(struct VEHICLE_DATA* data, int index)
{
    ParBuffer inBuf;
    std::string v;
    inBuf << CC_PAR_VEHICLE_DATA << index;
    veinsVehicle().getParameter(inBuf.str(), v);
    ParBuffer outBuf(v);
    outBuf >> data->index >> data->speed >> data->acceleration >> data->positionX >> data->positionY >> data->time >> data->length >> data->u >> data->speedX >> data->speedY >> data->angle;
}

void CommandInterface::Vehicle::useControllerAcceleration(bool use)
{
    veinsVehicle().setParameter(PAR_USE_CONTROLLER_ACCELERATION, use ? 1 : 0);
}

void CommandInterface::Vehicle::getEngineData(int& gear, double& rpm)
{
    ParBuffer inBuf;
    std::string v;
    inBuf << PAR_ENGINE_DATA;
    veinsVehicle().getParameter(inBuf.str(), v);
    ParBuffer outBuf(v);
    outBuf >> gear >> rpm;
}

void CommandInterface::Vehicle::enableAutoFeed(bool enable, std::string leaderId, std::string frontId)
{
    if (enable && (leaderId.compare("") == 0 || frontId.compare("") == 0)) return;
    ParBuffer inBuf;
    if (enable)
        inBuf << 1 << leaderId << frontId;
    else
        inBuf << 0;
    veinsVehicle().setParameter(PAR_USE_AUTO_FEEDING, inBuf.str());
}

void CommandInterface::Vehicle::usePrediction(bool enable)
{
    veinsVehicle().setParameter(PAR_USE_PREDICTION, enable ? 1 : 0);
}

void CommandInterface::Vehicle::addPlatoonMember(std::string memberId, int position)
{
    ParBuffer inBuf;
    inBuf << memberId << position;
    veinsVehicle().setParameter(PAR_ADD_MEMBER, inBuf.str());
}

void CommandInterface::Vehicle::removePlatoonMember(std::string memberId)
{
    veinsVehicle().setParameter(PAR_REMOVE_MEMBER, memberId);
}

void CommandInterface::Vehicle::enableAutoLaneChanging(bool enable)
{
    veinsVehicle().setParameter(PAR_ENABLE_AUTO_LANE_CHANGE, enable ? 1 : 0);
}

void CommandInterface::Vehicle::performPlatoonLaneChange(int lane)
{
    veinsVehicle().setParameter(PAR_PLATOON_FIXED_LANE, lane);
}

unsigned int CommandInterface::Vehicle::getLanesCount()
{
    int v;
    veinsVehicle().getParameter(PAR_LANES_COUNT, v);
    return (unsigned int) v;
}

void CommandInterface::vehicleRemoved(cObject* module, std::string platooningVType)
{
    cModule* mod = dynamic_cast<cModule*>(module);
    ASSERT(mod);
    auto mobilityModules = veins::getSubmodulesOfType<veins::TraCIMobility>(mod);
    veins::TraCIMobility* mob = mobilityModules[0];
    ASSERT(mob);

    std::string sumo_id = mob->getExternalId();
    // if this is not a platooning vehicle simply ignore its deletion
    if (sumo_id.find(platooningVType) == std::string::npos)
        return;

    // Delete the whole platoon when leader vanishes
    BasePositionHelper* positionHelper = veins::FindModule<BasePositionHelper*>::findSubModule(mod);
    veins::TraCIScenarioManager* manager = veins::TraCIScenarioManagerAccess().get();
    double maxObservedVehicles = manager->par("maxObservedVehicles").intValue();

    killedVehicles++; // count killed leaders/non-followers...
    int progress = (int)(killedVehicles / maxObservedVehicles * 100);
    if (progress > 0 && progress % 10 == 0) {
        std::cout << " Sim Progress: " << killedVehicles / maxObservedVehicles * 100
            << "%" << " observedVehs: " << killedVehicles << " out of necessary: " << maxObservedVehicles << std::endl;
    }
    ASSERT(positionHelper);
    if (positionHelper->isLeader()) {
        EV_DEBUG << "Leader " << sumo_id << " (Plexe id " << positionHelper->getId() << ") is being removed from the simulation. Removing followers as well";
        auto form = positionHelper->getPlatoonFormation();
        for (int i = 1; i < form.size(); i++) {
            std::stringstream ss;
            ss << platooningVType << "." << form[i];
            std::string follower_id = ss.str();

            EV_DEBUG << "Removing follower " << follower_id;
            manager->removeVehicle(follower_id, false);
            killedVehicles++; // count killed followers
            if (progress > 0 && progress % 10 == 0) {
                std::cout << " Sim Progress: " << killedVehicles / maxObservedVehicles * 100
                    << "%" << " observedVehs: " << killedVehicles << " out of necessary: " << maxObservedVehicles << std::endl;
            }
            ss.clear();
        }
    }

    if (killedVehicles >= maxObservedVehicles) {
        std::cout << "Simulation ended because " << killedVehicles << " has left already the simulation" << std::endl;
        manager->endSimulation();
    }
}

} // namespace traci
} // namespace plexe
