//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2020 Piermaria Arvani <piermaria.arvani@gmail.com>
//
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

import org.car2x.veins.base.connectionManager.ConnectionManager;
import org.car2x.veins.base.modules.BaseWorldUtility;
import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerForker;
import org.car2x.veins.modules.obstacle.ObstacleControl;
import org.car2x.veins.modules.world.annotations.AnnotationManager;

import org.car2x.plexe.PlexeManager;
import org.car2x.plexe.mobility.TraCIBaseTrafficManager;

import org.car2x.veins.modules.obstacle.VehicleObstacleControl;
import org.car2x.veinsvlc.VlcConnectionManager;

network PlatooningVlc
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        string traffic_type;
        @display("bgb=$playgroundSizeX,$playgroundSizeY");
    submodules:
        obstacles: ObstacleControl {
            @display("p=240,50");
        }
        annotations: AnnotationManager {
            @display("p=260,50");
        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=150,0;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=30,0;i=misc/globe");
        }
        manager: TraCIScenarioManagerForker {
            parameters:
                @display("p=512,128");
        }
        plexe: PlexeManager {
            @display("p=280,50");
        }
        traffic: <traffic_type> like TraCIBaseTrafficManager {
            parameters:
                @display("p=200,200");
        }
        vehicleObstacles: VehicleObstacleControl {
            @display("p=261,35");
        }
        vlcConnectionManager: VlcConnectionManager {
            parameters:
                @display("p=146,112;i=abstract/multicast");
        }

    connections allowunconnected:
}

