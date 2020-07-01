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

package org.car2x.plexe.subprojects.plexe_lte;

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


module PlatoonCarLTE
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
        int numTcpApps = default(0);
        int numUdpApps = default(0);

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
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=50,125;is=s");
        }
        mobility: <mobilityType> like IMobility {
            @display("p=50,175;is=s");
        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            @display("p=177,112,row");
        }
        tcp: TCP if numTcpApps>0 {
            @display("p=177,212");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            @display("p=327,112,row");
        }
        udp: UDP {
            @display("p=327,212");
        }
        lteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            d2dCapable = d2dCapable;
            @display("p=252,469");
        }
        lteCV2XMode3Driver: LTECV2XMode3RadioDriver {
            @display("p=250.98,111.072,row");
            destinationPort = 3000;
            multicastAddress = "224.0.0.1";
        }
        // network layer
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=252,320");
        }
        configurator: HostAutoConfigurator {
            @display("p=49.068,22.968");
        }

    connections allowunconnected:

        // LTE stack connections
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.transportIn++ if numTcpApps>0;
        tcp.ipIn <-- networkLayer.transportOut++ if numTcpApps>0;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

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


}
