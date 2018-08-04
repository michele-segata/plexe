//
// Copyright (C) 2012-2018 Michele Segata <segata@ccs-labs.org>
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

package org.car2x.veins.modules.application.platooning.scenarios;

import org.car2x.veins.base.modules.IBaseApplLayer;

moduleinterface BaseScenario extends IBaseApplLayer
{
    parameters:

        //controller used by followers
        string controller;
        //list of various controller parameters
        //headway time to be used for the ACC
        double accHeadway @unit("s");
        //headway time for leaders using ACC
        double leaderHeadway @unit("s");
        //cacc and engine related parameters
        double caccXi;
        double caccOmegaN @unit("Hz");
        double caccC1;
        double engineTau @unit("s");
        double uMin @unit("mpsps");
        double uMax @unit("mpsps");
        double ploegH @unit("s");
        double ploegKp;
        double ploegKd;
        double flatbedKa;
        double flatbedKv;
        double flatbedKp;
        double flatbedH;
        double flatbedD;
        //use the real or the controller acceleration for computing the acceleration
        bool useControllerAcceleration;
        //use prediction for computing missing data between successive beacons
        bool usePrediction;

        //location of the file with vehicle parameters
        string vehicleFile;
        //enable/disable the realistic engine model
        bool useRealisticEngine;
        //vehicle type for the realistic engine model
        string vehicleType;

        @display("i=block/app2");
        @class(BaseScenario);

}
