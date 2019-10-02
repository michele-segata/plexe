//
// Copyright (C) 2013-2019 Michele Segata <segata@ccs-labs.org>, Stefan Joerer <joerer@ccs-labs.org>
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

#include "plexe/mobility/TraCIBaseTrafficManager.h"

using namespace veins;

namespace plexe {

Define_Module(TraCIBaseTrafficManager);

void TraCIBaseTrafficManager::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0) {

        // empty all vectors
        vehicleTypeIds.clear();
        vehiclesCount.clear();
        laneIds.clear();
        roadIds.clear();
        routeIds.clear();
        laneIdsOnEdge.clear();
        routeStartLaneIds.clear();
        vehicleInsertQueue.clear();

        insertInOrder = true;

        // search for the scenario manager. it will be needed to inject vehicles
        manager = FindModule<veins::TraCIScenarioManager*>::findGlobalModule();
        ASSERT2(manager, "cannot find TraciScenarioManager");

        // reset vehicles counter
        vehCounter = 0;

        // subscribe to signals
        auto init = [this](veins::SignalPayload<bool>) { loadSumoScenario(); };
        signalManager.subscribeCallback(manager, veins::TraCIScenarioManager::traciInitializedSignal, init);
    }
}

void TraCIBaseTrafficManager::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    }
}

int TraCIBaseTrafficManager::findVehicleTypeIndex(std::string vehType)
{

    unsigned int i;

    for (i = 0; i < vehicleTypeIds.size(); i++) {
        if (vehicleTypeIds[i].compare(vehType) == 0) {
            return i;
        }
    }

    return -1;
}

void TraCIBaseTrafficManager::loadSumoScenario()
{
    commandInterface = manager->getCommandInterface();

    // get all the vehicle types
    if (vehicleTypeIds.size() == 0) {
        std::list<std::string> vehTypes = commandInterface->getVehicleTypeIds();
        EV << "Having currently " << vehTypes.size() << " vehicle types" << std::endl;
        for (std::list<std::string>::const_iterator i = vehTypes.begin(); i != vehTypes.end(); ++i) {
            if (i->compare("DEFAULT_VEHTYPE") != 0) {
                EV << "found vehType " << (*i) << std::endl;
                vehicleTypeIds.push_back(*i);
                // set counter of vehicles for this vehicle type to 0
                vehiclesCount.push_back(0);
            }
        }
    }
    // get all roads
    if (roadIds.size() == 0) {
        std::list<std::string> roads = commandInterface->getRoadIds();
        EV << "Having currently " << roads.size() << " roads in the scenario" << std::endl;
        for (std::list<std::string>::const_iterator i = roads.begin(); i != roads.end(); ++i) {
            EV << *i << std::endl;
            roadIds.push_back(*i);
        }
    }
    // get all lanes
    if (laneIds.size() == 0) {
        std::list<std::string> lanes = commandInterface->getLaneIds();
        EV << "Having currently " << lanes.size() << " lanes in the scenario" << std::endl;
        for (std::list<std::string>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
            EV << *i << std::endl;
            laneIds.push_back(*i);
            std::string edgeId = commandInterface->lane(*i).getRoadId();
            laneIdsOnEdge[edgeId].push_back(*i);
        }
    }
    // get all routes
    if (routeIds.size() == 0) {
        std::list<std::string> routes = commandInterface->getRouteIds();
        EV << "Having currently " << routes.size() << " routes in the scenario" << std::endl;
        for (std::list<std::string>::const_iterator i = routes.begin(); i != routes.end(); ++i) {
            std::string routeId = *i;
            EV << routeId << std::endl;
            routeIds.push_back(routeId);
            std::list<std::string> routeEdges = commandInterface->route(routeId).getRoadIds();
            std::string firstEdge = *(routeEdges.begin());
            EV << "First Edge of route " << routeId << " is " << firstEdge << std::endl;
            routeStartLaneIds[routeId] = laneIdsOnEdge[firstEdge];
        }
    }
    // inform inheriting classes that scenario is loaded
    scenarioLoaded();

    auto timestep = [this](veins::SignalPayload<simtime_t const&>) { insertVehicles(); };
    signalManager.subscribeCallback(manager, veins::TraCIScenarioManager::traciTimestepEndSignal, timestep);
}

void TraCIBaseTrafficManager::insertVehicles()
{
    // insert the vehicles in the queue
    for (InsertQueue::iterator i = vehicleInsertQueue.begin(); i != vehicleInsertQueue.end(); ++i) {
        std::string route = routeIds[i->first];
        EV << "process " << route << std::endl;
        std::deque<struct Vehicle>::iterator vi = i->second.begin();
        while (vi != i->second.end() && i->second.size() != 0) {
            bool suc = false;
            struct Vehicle v = *vi;
            std::string type = vehicleTypeIds[v.id];
            std::stringstream veh;
            veh << type << "." << vehiclesCount[v.id];

            // do we need to put this vehicle on a particular lane, or can we put it on any?

            if (v.lane == -1 && !insertInOrder) {

                // try to insert that into any lane
                for (unsigned int laneId = 0; !suc && laneId < routeStartLaneIds[route].size(); laneId++) {
                    EV << "trying to add " << veh.str() << " with " << route << " vehicle type " << type << std::endl;
                    suc = commandInterface->addVehicle(veh.str(), type, route, simTime(), v.position, v.speed, laneId);
                    if (suc) break;
                }
                if (!suc) {
                    // if we did not manager to insert a car on any lane, then this route is full and we can just stop
                    // TODO: this is not true if we want to insert a vehicle not at the beginning of the route. fix this
                    break;
                }
                else {
                    EV << "successful inserted " << veh.str() << std::endl;
                    vi = i->second.erase(vi);
                    vehiclesCount[v.id] = vehiclesCount[v.id] + 1;
                }
            }
            else {

                // try to insert into desired lane
                EV << "trying to add " << veh.str() << " with " << route << " vehicle type " << type << std::endl;
                suc = commandInterface->addVehicle(veh.str(), type, route, simTime(), v.position, v.speed, v.lane);

                if (suc) {
                    EV << "successful inserted " << veh.str() << std::endl;
                    vi = i->second.erase(vi);
                    vehiclesCount[v.id] = vehiclesCount[v.id] + 1;
                }
                else {
                    if (!insertInOrder) {
                        vi++;
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }
}

void TraCIBaseTrafficManager::addVehicleToQueue(int routeId, struct Vehicle v)
{
    vehicleInsertQueue[routeId].push_back(v);
}

} // namespace plexe
