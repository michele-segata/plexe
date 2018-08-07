//
// Copyright (C) 2013-2018 Michele Segata <segata@ccs-labs.org>, Stefan Joerer <joerer@ccs-labs.org>
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

#ifndef TRACIBASETRAFFICMANAGER_H_
#define TRACIBASETRAFFICMANAGER_H_

#include <omnetpp.h>
#include <queue>
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

class TraCIBaseTrafficManager : public cSimpleModule {

public:
    virtual void initialize(int stage);
    virtual void finish();

    int findVehicleTypeIndex(std::string vehType);

public:
    TraCIBaseTrafficManager()
    {
        insertVehiclesTrigger = 0;
    }

private:
    /**
     * Loads data about vehicles, routes, etc...
     */
    void loadSumoScenario();

    // total number of vehicles generated
    int vehCounter;
    // has the data about the scenario been loaded?
    bool initScenario;
    // should vehicles be inserted in order, or whenever there is room for doing so?
    bool insertInOrder;

    // at each simulation step, triggers insertion of vehicles in the queue
    cMessage* insertVehiclesTrigger;

protected:
    // pointer to the scenario manager, used to issue traci commands
    Veins::TraCIScenarioManager* manager;
    // pointer to the command interface
    Veins::TraCICommandInterface* commandInterface;
    // vector of all the vehicle types available
    std::vector<std::string> vehicleTypeIds;
    // vector counting the number of vehicles inserted per each type
    std::vector<int> vehiclesCount;
    // vector of all lanes ids
    std::vector<std::string> laneIds;
    // vector of all the roads ids
    std::vector<std::string> roadIds;
    // vector of all the routes ids
    std::vector<std::string> routeIds;
    // mapping between the edge id and the ids of the lanes in that edge
    std::map<std::string, std::vector<std::string>> laneIdsOnEdge;
    // mapping between the route id and the ids of the lanes at the start of the route
    std::map<std::string, std::vector<std::string>> routeStartLaneIds;

    struct Vehicle {
        int id; // id of the vehicle in sumo. this is the index of the vehicle type in the array of vehicle types
        int lane; // index of the lane where to insert (set to -1 to choose first free)
        float position; // position on the first edge
        float speed; // start speed (-1 for lane speed?)
    };

    // queue of vehicles to be inserted. maps the index of a route in routeIds to a list of indexes of vehicle
    // types in vehicleTypeIds
    typedef std::map<int, std::deque<struct Vehicle>> InsertQueue;

private:
    InsertQueue vehicleInsertQueue;

protected:
    void addVehicleToQueue(int routeId, struct Vehicle v);

    /**
     * Inserts the vehicles which have been put into the queue
     */
    void insertVehicles();

    virtual void handleSelfMsg(cMessage* msg);
    virtual void handleMessage(cMessage* msg);

    /**
     * virtual function that inheriting classes can override to get informed when scenario is loaded
     */
    virtual void scenarioLoaded(){};
};

#endif /* TRACIBASETRAFFICMANAGER_H_ */
