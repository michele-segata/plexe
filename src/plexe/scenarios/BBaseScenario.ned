//
// Copyright (C) 2012-2019 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
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

package org.car2x.plexe.scenarios;

import org.car2x.plexe.scenarios.BaseScenario;

simple BBaseScenario like BaseScenario
{
    parameters:

        //average speed
        double leaderSpeed @unit("kmph") = default(100kmph);

        //base scenario inherited parameters
        //controller used by followers
        string controller;
        //list of various controller parameters
        //headway time to be used for the ACC
        double accHeadway @unit("s") = default(1.2s);
        //headway time for leaders using ACC
        double leaderHeadway @unit("s") = default(1.2s);
        //cacc and engine related parameters
        double caccXi = default(1);
        double caccOmegaN @unit("Hz") = default(0.2Hz);
        double caccC1 = default(0.5);
        double caccSpacing @unit("m") = default(5m);
        double engineTau @unit("s") = default(0.5s);
        double uMin @unit("mpsps") = default(-1e6 mpsps);
        double uMax @unit("mpsps") = default(1e6 mpsps);
        double ploegH @unit("s") = default(0.5s);
        double ploegKp = default(0.2);
        double ploegKd = default(0.7);
        double flatbedKa = default(2.4);
        double flatbedKv = default(0.6);
        double flatbedKp = default(12);
        double flatbedH = default(4);
        double flatbedD = default(5);
        bool useControllerAcceleration = default(true);
        bool usePrediction = default(true);

        //location of the file with vehicle parameters
        string vehicleFile = default("./vehicles.xml");
        //enable/disable the realistic engine model
        bool useRealisticEngine = default(false);
        //vehicle type for the realistic engine model
        string vehicleType = default("");

        int headerLength @unit("bit") = default(0bit);

    gates:
        input lowerLayerIn; // from network layer
        output lowerLayerOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer

}
