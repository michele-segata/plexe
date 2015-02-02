//
// Copyright (C) 2012-2015 Michele Segata <segata@ccs-labs.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package org.car2x.veins.modules.application.platooning.apps;

import org.car2x.veins.modules.application.platooning.apps.BaseApp;

simple SimplePlatooningApp like BaseApp
{
    parameters:
        double leaderOscillationFrequency @unit("Hz") = default(0.1 Hz);
        int headerLength @unit("bit") = default(0 bit);
        //defined the controller to be used for platooning
        string controller @enum("ACC", "CACC") = default("CACC");
        //headway time to be used for the ACC
        double accHeadway @unit("s") = default(1.5s);
        //send actual or controller-computed acceleration
        bool useControllerAcceleration = default(true);
                //cacc and engine related parameters
        double caccXi = default(1);
        double caccOmegaN @unit("Hz") = default(0.2 Hz);
        double caccC1 = default(0.5);
        double engineTau @unit("s") = default(0.5 s);
        double ploegH @unit("s") = default(0.5 s);
        double ploegKp = default(0.2);
        double ploegKd = default(0.7);

        //leader average speed during platooning
        double leaderSpeed @unit("kmph") = default(150kmph);

        @display("i=block/app2");
        @class(SimplePlatooningApp);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}
