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

import org.car2x.plexe.subprojects.plexe_5g.UEBaseApp;


 //
 //  This is a UE app that asks to a Device App to instantiate the MECWarningAlertApp.
 //  After a successful response, it asks to the MEC app to be notified when the car
 //  enters a circle zone described by x,y center position and the radius. When a danger
 //  event arrives, the car colors becomes red.
 //
 //  The event behavior flow of the app is:
 //  1) send create MEC app to the Device App
 //  2.1) ACK --> send coordinates to MEC app
 //  2.2) NACK --> do nothing
 //  3) wait for danger events
 //  4) send delete MEC app to the Device App
 //
simple UEOvertakeApp extends UEBaseApp
{
    parameters:
        @class(plexe::UEOvertakeApp);
        @display("i=block/source");

        //autoscheduling infos
        double startTime @unit("s") = default(0s);
        double stopTime @unit("s") = default(120s);

        double overtakeSpeed @unit("mps");
        double overtakeAcceleration @unit("mpsps");

        // indicates how frequently update messages should be sent to the MEC app
        double sendUpdateInterval @unit(s);
        double sendStartOvertakeTime @unit(s);

        // specifies the ue app id for the application
        int deviceAppId;
}

