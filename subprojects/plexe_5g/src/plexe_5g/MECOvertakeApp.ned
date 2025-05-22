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

simple MECOvertakeApp extends MECBaseApp
{
    parameters:
        @class(plexe::MECOvertakeApp);
        @display("i=block/app;is=s");

        double rho;
        double epsilon @unit(m);
        bool useCooperativeOvertake;
        // target speed of incoming vehicle
        double v3 @unit(mps);

        // name of the migrator module
        string migrator;

}

