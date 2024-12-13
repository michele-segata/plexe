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

import org.car2x.plexe.subprojects.plexe_5g.MECBaseApp;


simple MECTrafficAuthorityApp extends MECBaseApp
{
    parameters:
        @class(plexe::MECTrafficAuthorityApp);
        @display("i=block/app;is=s");

        // distance from the approached platoon at which the TA stops commanding the approaching platoon
        double approachDistanceThreshold @unit(m) = default(200m);
        // speed difference to be used to approach a platoon
        double approachSpeedDelta @unit(mps) = default(10mps);

}

