//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
// Copyright (C) 2020 Piermaria Arvani <piermaria.arvani@gmail.com>
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

package org.car2x.plexe.subprojects.plexe_hetnet;

import org.car2x.veins.base.modules.IBaseApplLayer;
import org.car2x.veins.modules.mobility.traci.TraCIMobility;
import org.car2x.veins.modules.nic.Nic80211p;
import inet.mobility.contract.IMobility;
import org.car2x.veins.subprojects.veins_inet.VeinsInetMobility;

import org.car2x.plexe.utilities.BasePositionHelper;
import org.car2x.plexe.scenarios.BaseScenario;
import org.car2x.plexe.protocols.BaseProtocol;
import org.car2x.plexe.apps.BaseApp;
import org.car2x.plexe.driver.Veins11pRadioDriver;

import inet.applications.contract.ITCPApp;
import inet.applications.contract.IUDPApp;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;
import inet.transportlayer.tcp.TCP;
import inet.transportlayer.udp.UDP;
import lte.stack.ILteNic;
import lte.stack.LteNicUe;
import lte.stack.LteNicUeD2D;
import org.car2x.plexe.subprojects.plexe_lte.LTECV2XMode3RadioDriver;

import org.car2x.plexe.subprojects.plexe_vlc.VeinsVLCRadioDriver;
import org.car2x.veinsvlc.NicVlc;
import org.car2x.veinsvlc.Splitter;

module PlatoonCarHetNet
{
    parameters:
        @display("p=90,100;bgb=468.852,715.56");
        @networkNode;
        string scenario_type;
        string helper_type;
        string appl_type;
        string protocol_type;

        string mobilityType = default("VeinsInetMobility");

        //# Node specs
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId;
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter

        //# D2D capability
        bool d2dCapable = default(true);
        string nicType = d2dCapable ? "LteNicUeD2D" : "LteNicUe";

        //# Network Layer specs
        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));


    gates:
        input radioIn @directIn; // gate for sendDirect
        input headLightIn;
        input tailLightIn;
    submodules:

        helper: <helper_type> like BasePositionHelper {
            parameters:
                @display("p=204,30");
        }

        scenario: <scenario_type> like BaseScenario {
            parameters:
                @display("p=129,30");
        }

        appl: <appl_type> like BaseApp {
            parameters:
                @display("p=216.804,158.064");
        }

        prot: <protocol_type> like BaseProtocol {
            parameters:
                @display("p=216.804,233.892");
        }

        veins11pDriver: Veins11pRadioDriver {
            parameters:
                @display("p=59.808,390.888");
        }

        nic: Nic80211p {
            parameters:
                @display("p=59.808,649.344");
        }

        veinsmobility: TraCIMobility {
            parameters:
                @display("p=48.059998,233.892;i=block/cogwheel");
        }

        interfaceTable: InterfaceTable {
            @display("p=50,75;is=s");
        }
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=50,125;is=s");
        }
        mobility: <mobilityType> like IMobility {
            @display("p=50,175;is=s");
        }
        udp: UDP {
            @display("p=384.47998,481.668");
        }
        lteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            d2dCapable = d2dCapable;
            @display("p=384.47998,649.344");
        }
        lteCV2XMode3Driver: LTECV2XMode3RadioDriver {
            @display("p=384.47998,390.888,row");
            destinationPort = 3000;
            multicastAddress = "224.0.0.1";
        }
        // network layer
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=384.47998,567.108");
        }
        configurator: HostAutoConfigurator {
            @display("p=49.068,22.968");
        }

        veinsVlcDriver: VeinsVLCRadioDriver {
            parameters:
                @display("p=216.804,390.888");
        }

        nicVlcHead: NicVlc {
            parameters:
                @display("p=269.136,649.344");
        }

        nicVlcTail: NicVlc {
            parameters:
                @display("p=164.472,649.344");
        }

        splitter: Splitter {
            @display("p=216.804,567.108");
        }

    connections allowunconnected:

        // LTE stack connections
        udp.ipOut --> networkLayer.transportIn++;
        udp.ipIn <-- networkLayer.transportOut++;

        lteNic.radioIn <-- radioIn;

        networkLayer.ifOut++ --> lteNic.upperLayerIn;
        networkLayer.ifIn++ <-- lteNic.upperLayerOut;

        //11p stack connections
        nic.upperLayerIn <-- veins11pDriver.lowerLayerOut;
        nic.upperLayerOut --> veins11pDriver.lowerLayerIn;
        veins11pDriver.upperLayerIn <-- prot.radiosOut++;
        veins11pDriver.upperLayerOut --> prot.radiosIn++;
        //LTE D2D connections
        lteCV2XMode3Driver.upperLayerIn <-- prot.radiosOut++;
        lteCV2XMode3Driver.upperLayerOut --> prot.radiosIn++;
        lteCV2XMode3Driver.udpOut --> udp.appIn++;
        lteCV2XMode3Driver.udpIn <-- udp.appOut++;
        //VLC connections
        veinsVlcDriver.upperLayerIn <-- prot.radiosOut++;
        veinsVlcDriver.upperLayerOut --> prot.radiosIn++;

        splitter.applicationIn <-- veinsVlcDriver.lowerLayerOut;
        splitter.applicationOut --> veinsVlcDriver.lowerLayerIn;

        splitter.nicVlcHeadOut --> nicVlcHead.upperLayerIn;
        splitter.nicVlcHeadIn <-- nicVlcHead.upperLayerOut;

        splitter.nicVlcTailOut --> nicVlcTail.upperLayerIn;
        splitter.nicVlcTailIn <-- nicVlcTail.upperLayerOut;

        headLightIn --> nicVlcHead.radioIn;
        tailLightIn --> nicVlcTail.radioIn;


}
