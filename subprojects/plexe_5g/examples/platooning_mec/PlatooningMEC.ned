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

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth10G;

import simu5g.world.radio.LteChannelControl;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.Upf;
import simu5g.common.binder.Binder;
import simu5g.nodes.NR.gNodeB;

import simu5g.nodes.mec.MECHost;
import simu5g.nodes.mec.MECOrchestrator.MecOrchestrator;
import simu5g.nodes.mec.UALCMP.UALCMP;

import org.car2x.plexe.subprojects.plexe_5g.PlatoonCarNR;

import org.car2x.veins.subprojects.veins_inet.VeinsInetManagerForker;

network PlatooningMEC
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        string traffic_type;
        @display("bgb=1163.62,658.45996");
    submodules:
        //# 5G modules
        routingRecorder: RoutingTableRecorder {
            @display("p=258.41998,551.88;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=159.14,551.88");
            config = xmldoc("demo.xml");
        }
        channelControl: LteChannelControl {
            @display("p=255.5,624.88;is=s");
        }
        binder: Binder {
            @display("p=468.66,562.1;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=50.993748,258.7;is=s");
        }
        router: Router {
            @display("p=576.20624,392.40375;i=device/smallrouter");
        }
        trafficAuthority: StandardHost {
            @display("p=691.4475,449.29498;is=n;i=device/server");
        }
        upf: Upf {
            @display("p=468.66,394.19998");
        }
        iUpf1: Upf {
            @display("p=320.925,258.19873");
        }
        iUpf2: Upf {
            @display("p=691.4475,256.74");
        }
        gnb1: gNodeB {
            @display("p=159.00374,97.736244;is=vl");
        }
        gnb2: gNodeB {
            @display("p=927.76495,97.736244;is=vl");
        }

        //# MEC modules
        mecHost1: MECHost {
            @display("p=208.60124,258.19873");
        }
        mecHost2: MECHost {
            @display("p=806.6887,256.74");
        }
        mecOrchestrator: MecOrchestrator {
            @display("p=830.02875,336.97125");
        }
        ualcmp: UALCMP {
            @display("p=691.4475,336.97125");
        }

        obstacles: ObstacleControl {
            @display("p=49.64,461.36");
        }
        annotations: AnnotationManager {
            @display("p=49.64,553.33997");
        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=49.64,639.48;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = parent.playgroundSizeX;
                playgroundSizeY = parent.playgroundSizeY;
                playgroundSizeZ = parent.playgroundSizeZ;
                @display("p=376.68,562.1;i=misc/globe");
        }
        manager: VeinsInetManagerForker {
            parameters:
                @display("p=512,128");
        }
        plexe: PlexeManager {
            @display("p=49.64,365");
        }
        traffic: <traffic_type> like TraCIBaseTrafficManager {
            parameters:
                @display("p=567.94,573.77997");
        }
        node[0]: PlatoonCarNR {
            @display("p=10.211249,198.39");
        }
    connections allowunconnected:
        upf.pppg++ <--> Eth10G <--> iUpf1.pppg++;
        upf.pppg++ <--> Eth10G <--> iUpf2.pppg++;
        iUpf1.pppg++ <--> Eth10G <--> gnb1.ppp;
        iUpf2.pppg++ <--> Eth10G <--> gnb2.ppp;

                //# MEC-related connections
        //        ualcmp.ppp++ <--> Eth10G <--> upf.filterGate;
        ualcmp.ppp++ <--> Eth10G <--> router.pppg++;
        ualcmp.toMecOrchestrator --> mecOrchestrator.fromUALCMP;
        ualcmp.fromMecOrchestrator <-- mecOrchestrator.toUALCMP;
        mecHost1.ppp++ <--> Eth10G <--> iUpf1.pppg++;
        mecHost2.ppp++ <--> Eth10G <--> iUpf2.pppg++;

        trafficAuthority.pppg++ <--> Eth10G <--> router.pppg++;
        //# X2 connections
        gnb1.x2++ <--> Eth10G <--> gnb2.x2++;
        router.pppg++ <--> Eth10G <--> upf.filterGate;

}

