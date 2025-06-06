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

import lte.corenetwork.nodes.eNodeB;
import inet.node.ethernet.Eth10G;
import lte.corenetwork.binder.LteBinder;
import lte.world.radio.LteChannelControl;
import lte.common.LteNetworkConfigurator;
import lte.epc.PgwStandardSimplified;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

import org.car2x.veins.subprojects.veins_inet.VeinsInetManagerForker;

network PlatooningLte
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        string traffic_type;
        @display("bgb=1164.825,450.9");
    submodules:
        //# LTE modules
        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        configurator: LteNetworkConfigurator {
            @display("p=50,125");
            config = xmldoc("demo.xml");
        }
        binder: LteBinder {
            @display("p=50,175;is=s");
        }
        router: Router {
            @display("p=321,136;i=device/smallrouter");
        }
        trafficAuthority: StandardHost {
            @display("p=212,118;is=n;i=device/server");
        }
        pgw: PgwStandardSimplified {
            nodeType = "PGW";
            @display("p=172,287;is=l");
        }
        eNodeB1: eNodeB {
            @display("p=0,0;is=vl");
        }
        eNodeB2: eNodeB {
            @display("p=1000,0;is=vl");
        }
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
        manager: VeinsInetManagerForker {
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

    connections allowunconnected:
        trafficAuthority.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> pgw.filterGate;
        pgw.pppg++ <--> Eth10G <--> eNodeB1.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB2.ppp;
        eNodeB1.x2++ <--> Eth10G <--> eNodeB2.x2++;
}

