//
// Copyright (C) 2008-2025 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
// Copyright (C) 2020-2025 Piermaria Arvani <piermaria.arvani@gmail.com>
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

import inet.applications.contract.IApp;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.tcp.Tcp;
import inet.transportlayer.udp.Udp;
import inet.common.MessageDispatcher;
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
        @display("p=90,100;bgb=403,530");
        @networkNode;
        string scenario_type;
        string helper_type;
        string appl_type;
        string protocol_type;

        string mobilityType = default("VeinsInetMobility");

        //# Apps
        int numApps = default(0);

        //# Node specs
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId;
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter

        //# D2D capability
        bool d2dCapable = default(true);
        string nicType = d2dCapable ? "LteNicUeD2D" : "LteNicUe";

        //# Network Layer specs
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));

        bool enableExtInterface = default(false);


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
                @display("p=60,234");
        }

        prot: <protocol_type> like BaseProtocol {
            parameters:
                @display("p=60,308");
        }

        veins11pDriver: Veins11pRadioDriver {
            parameters:
                @display("p=60,387");
        }

        nic: Nic80211p {
            parameters:
                @display("p=60,469");
        }

        veinsmobility: TraCIMobility {
            parameters:
                @display("p=138,308;i=block/cogwheel");
        }

        interfaceTable: InterfaceTable {
            @display("p=50,75;is=s");
        }
        mobility: <mobilityType> like IMobility {
            @display("p=50,175;is=s");
        }
        lteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            d2dCapable = d2dCapable;
            @display("p=252,469");
        }
        // network layer
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer {
            parameters:
                @display("p=252,320");
        }
        tcp: Tcp if numApps>0 {
            @display("p=177,212");
        }
        udp: Udp {
            @display("p=327,212");
        }
        app[numApps]: <> like IApp {
            @display("p=177,112,row");
        }
        at: MessageDispatcher {
            parameters:
                @display("p=750,146;b=1000,5,,,,1");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=750,146;b=1000,5,,,,1");
        }
        nl: MessageDispatcher {
            parameters:
                @display("p=750,146;b=1000,5,,,,1");
        }
        lteCV2XMode3Driver: LTECV2XMode3RadioDriver {
            @display("p=250.98,111.072,row");
            destinationPort = 3000;
            multicastAddress = "224.0.0.1";
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
        // app to tcp and udp
        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }

        at.out++ --> tcp.appIn if numApps > 0;
        at.in++ <-- tcp.appOut if numApps > 0;

        at.out++ --> udp.appIn;
        at.in++ <-- udp.appOut;

        //tcp and udp to network
        tcp.ipOut --> tn.in++ if numApps > 0;
        tcp.ipIn <-- tn.out++ if numApps > 0;

        udp.ipOut --> tn.in++;
        udp.ipIn <-- tn.out++;

        tn.out++ --> ipv4.transportIn;
        tn.in++ <-- ipv4.transportOut;

        //network to link
        nl.out++ --> ipv4.ifIn;
        nl.in++ <-- ipv4.ifOut;

        nl.out++ --> lteNic.upperLayerIn;
        nl.in++ <-- lteNic.upperLayerOut;

        lteNic.radioIn <-- radioIn;

        //11p stack connections
        nic.upperLayerIn <-- veins11pDriver.lowerLayerOut;
        nic.upperLayerOut --> veins11pDriver.lowerLayerIn;
        veins11pDriver.upperLayerIn <-- prot.radiosOut++;
        veins11pDriver.upperLayerOut --> prot.radiosIn++;
        //LTE D2D connections
        lteCV2XMode3Driver.upperLayerIn <-- prot.radiosOut++;
        lteCV2XMode3Driver.upperLayerOut --> prot.radiosIn++;
        lteCV2XMode3Driver.socketOut --> at.in++;
        lteCV2XMode3Driver.socketIn <-- at.out++;
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
