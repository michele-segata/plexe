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

simple UEIntersectionMergeApp extends UEBaseApp
{
    parameters:
        @class(plexe::UEIntersectionMergeApp);
        @display("i=block/source");

        //autoscheduling infos
        double startTime @unit("s") = default(0s);
        double stopTime @unit("s") = default(120s);

        //shared edge of the route
        string commonEdge;
        // distance threshold before requesting the merge
        double distanceThreshold @unit(m);
        // interval between distance to intersection evaluation
        volatile double distanceInterval @unit(seconds);
        double holdingPosition @unit(m);
        double holdingAcceleration @unit(mpsps);

        bool usePeriodicUpdates;
        double intersectionUpdateInterval @unit(s);

        string mecAppName;
}

