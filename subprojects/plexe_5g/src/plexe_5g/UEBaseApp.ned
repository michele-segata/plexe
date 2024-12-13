//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//


package org.car2x.plexe.subprojects.plexe_5g;

import inet.applications.contract.IApp;


 //
 //  This is a UE app that can be used as a base for building UE apps, extending its functionalities.
 //
 //  The event behavior flow of the app is:
 //  1) send create MEC app to the Device App
 //  2.1) ACK --> send coordinates to MEC app
 //  2.2) NACK --> do nothing
 //
simple UEBaseApp like IApp
{
    parameters:
        @class(plexe::UEBaseApp);
        @display("i=block/source");

        //connection infos
        int localPort = default(4000);
        int deviceAppPort = default(4000); // ipAddress of the DeviceApp
        string deviceAppAddress; // port of the DeviceApp

        int packetSize @unit(B) = default(10B);

        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value

        //autoscheduling infos
        double period @unit("s") = default(1s);

        bool logger = default(false);
        string interfaceTableModule;
        
        // multicast address used to communicate via 5G V2X sidelink
        string multicastAddress = default("224.0.0.1");
        int multicastPort = default(6789);

    gates:
        output socketOut;
        input socketIn;
        // input and output gates for 5G V2X multicast traffic
        input multicastSocketIn;
        output multicastSocketOut;
}

