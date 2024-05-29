//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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

package org.car2x.plexe.traci;

//
// Define an empty interface enabling to switch between Launchd and Forker
//
moduleinterface PlexeScenarioManager
{
    parameters:
        @display("i=block/network2");
        @signal[org_car2x_veins_modules_mobility_traciInitialized](type=bool);
        @signal[org_car2x_veins_modules_mobility_traciModulePreInit](type=cModule);
        @signal[org_car2x_veins_modules_mobility_traciModuleAdded](type=cModule);
        @signal[org_car2x_veins_modules_mobility_traciModuleRemoved](type=cModule);
        @signale[org_car2x_veins_modules_mobility_traciModuleUpdated](type=cModule);
        @signal[org_car2x_veins_modules_mobility_traciTimestepBegin](type=simtime_t);
        @signal[org_car2x_veins_modules_mobility_traciTimestepEnd](type=simtime_t);
        double connectAt @unit("s");  // when to connect to TraCI server (must be the initial timestep of the server)
        double firstStepAt @unit("s");  // when to start synchronizing with the TraCI server (-1: immediately after connecting)
        double updateInterval @unit("s");  // time interval of hosts' position updates
        string moduleType;  // module type to be used in the simulation for each managed vehicle
        string moduleName;  // module name to be used in the simulation for each managed vehicle
        // module displayString to be used in the simulation for each managed vehicle
        // display strings key-value pairs needs to be protected with single quotes, as they use an = sign as the type mappings. For example
        // <pre>
        // *.manager.moduleDisplayString = "'i=block/process'"
        // *.manager.moduleDisplayString = "a='i=block/process' b='i=misc/sun'"
        // </pre>
        //
        // moduleDisplayString can also be left empty:
        // <pre>
        // *.manager.moduleDisplayString = ""
        // </pre>
        string moduleDisplayString;
        string trafficLightModuleType;  // module type to be used in the simulation for each managed traffic light
        string trafficLightModuleName;  // module name to be used in the simulation for each managed traffic light
        string trafficLightFilter;  // filter string to select which tls shall be subscribed, list sumo IDs separated by spaces
        string trafficLightModuleDisplayString;  // module displayString to be used in the simulation for each managed traffic light
        string host;  // server hostname
        int port;  // server port (-1: automatic)
        int seed; // seed value to set in launch configuration, if missing (-1: current run number)
        bool autoShutdown;  // Shutdown module as soon as no more vehicles are in the simulation
        int margin;  // margin to add to all received vehicle positions
        string roiRoads;  // which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty
        string roiRects;  // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty. Note that these rectangles have to use TraCI (SUMO) coordinates and not OMNeT++. They can be easily read from sumo-gui.
        double penetrationRate; //the probability of a vehicle being equipped with Car2X technology
        bool ignoreGuiCommands; // whether to ignore all TraCI commands that only make sense when the server has a graphical user interface
        int order; // specific position in the multi-client execution order of the TraCI server to request upon connecting (-1: do not request a position)
        bool ignoreUnknownSubscriptionResults; // whether to (try and) ignore any subscription result we did not request (but another client might have)
}
