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

package org.car2x.plexe.subprojects.plexe_5g;

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
import inet.linklayer.contract.ILoopbackInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.tcp.Tcp;
import inet.transportlayer.udp.Udp;
import inet.common.MessageDispatcher;
import simu5g.stack.ICellularNic;
import org.car2x.plexe.subprojects.plexe_5g.NRCV2XMode1RadioDriver;


module PlatoonCarNR
{
    parameters:
        @display("p=90,100;bgb=883.23596,702.744");
        @networkNode;
        string scenario_type;
        string helper_type;
        string appl_type;
        string protocol_type;

        //# Mobility
        mobility.typename = default("VeinsInetMobility");

        //# Network Layer specs
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        *.interfaceTableModule = default(absPath(".interfaceTable"));

        ipv4.configurator.typename = "HostAutoConfigurator";
        ipv4.configurator.interfaces = "cellular lo0";

        //# Apps
        int numApps = default(0);   // no of apps.
                                    // Specify the app types in INI file with app[0..1].typename="UdpVideoStreamClient" syntax

        //# Transport layer
        bool hasUdp = default(firstAvailableOrEmpty("Udp") != "");
        bool hasTcp = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc") != "");

        //# Node specs
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId @mutable;
        int macNodeId @mutable = default(0); // TODO: this is not a real parameter
        int macCellId @mutable = default(0); // TODO: this is not a real parameter

        //# D2D capability
        string nicType = default("NRNicUe");
        int nrMasterId @mutable = default(0);
        int nrMacNodeId @mutable = default(0); // TODO: this is not a real parameter
        int nrMacCellId @mutable = default(0); // TODO: this is not a real parameter

        //# Network Layer specs
        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);

        //# External Ethernet interfaces
        string extHostAddress = default("");

        //# Loopback interfaces
        //CAREFUL: it is necessary to have a loopback interface as there might be internal dispatching of packets
        int numLoInterfaces = default(1);

        //# Num of PcapRecorders
        int numPcapRecorders = default(0); // no of PcapRecorders

        bool hasRNISupport = default(false);


    gates:
        input radioIn @directIn; // gate for sendDirect
        input nrRadioIn @directIn;
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

        traciMobility: TraCIMobility {
            parameters:
                @display("p=138,308;i=block/cogwheel");
        }

        interfaceTable: InterfaceTable {
            @display("p=50,75;is=s");
        }

        mobility: <default("")> like IMobility {
            @display("p=50,175;is=s");
        }

        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            parameters:
                @display("p=503.736,589.50397,row,150");
        }
        cellularNic: <nicType> like ICellularNic {
            nodeType = parent.nodeType;
            @display("p=355.152,589.50397");
        }
        // network layer
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4 {
            parameters:
                @display("p=375,376;q=queue");
        }
        //# transport layer
        udp: <default(firstAvailableOrEmpty("Udp"))> like IUdp if hasUdp {
            parameters:
                @display("p=375,226");
        }
        tcp: <default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc"))> like ITcp if hasTcp {
            parameters:
                @display("p=525,226");
        }

        //# app layer
        app[numApps]: <> like IApp {
            @display("p=374.47998,72.479996,row,140");
        }

        //# message dispatcher for SAP between application and transport layer
        at: MessageDispatcher {
            parameters:
                @display("p=550,146;b=600,5,,,,1");
        }
        //# message dispatcher for SAP between transport and network layer
        tn: MessageDispatcher {
            parameters:
                @display("p=550,296;b=600,5,,,,1");
        }
        //# message dispatcher for SAP to link layer
        nl: MessageDispatcher {
            parameters:
                @display("p=550,446;b=600,5,,,,1");
        }
        NRCV2XMode1Driver: NRCV2XMode1RadioDriver {
            @display("p=655.752,74.76,row");
            destinationPort = 3000;
            multicastAddress = "224.0.0.1";
        }

    connections allowunconnected:

        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        //#
        //# Internal transport layer connections to network layer
        //#

        udp.ipOut --> tn.in++ if hasUdp;
        udp.ipIn <-- tn.out++ if hasUdp;

        tcp.ipOut --> tn.in++ if hasTcp;
        tcp.ipIn <-- tn.out++ if hasTcp;

        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        ipv4.transportIn <-- tn.out++ if hasIpv4;
        ipv4.transportOut --> tn.in++ if hasIpv4;

        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;

        //#
        //# Internal link layer connections to network layer
        //#

        cellularNic.upperLayerOut --> nl.in++;
        cellularNic.upperLayerIn <-- nl.out++;

        for i=0..numLoInterfaces-1 {
            nl.out++ --> lo[i].upperLayerIn;
            lo[i].upperLayerOut --> nl.in++;
        }

        cellularNic.radioIn <-- radioIn;
        cellularNic.nrRadioIn <-- nrRadioIn;

        //11p stack connections
        nic.upperLayerIn <-- veins11pDriver.lowerLayerOut;
        nic.upperLayerOut --> veins11pDriver.lowerLayerIn;
        veins11pDriver.upperLayerIn <-- prot.radiosOut++;
        veins11pDriver.upperLayerOut --> prot.radiosIn++;
        //NR connections
        NRCV2XMode1Driver.upperLayerIn <-- prot.radiosOut++;
        NRCV2XMode1Driver.upperLayerOut --> prot.radiosIn++;
        NRCV2XMode1Driver.socketOut --> at.in++;
        NRCV2XMode1Driver.socketIn <-- at.out++;


}
