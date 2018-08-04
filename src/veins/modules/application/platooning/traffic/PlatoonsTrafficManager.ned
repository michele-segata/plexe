//
// Copyright (C) 2014-2018 Michele Segata <segata@ccs-labs.org>
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

package org.car2x.veins.modules.application.platooning.traffic;

import org.car2x.veins.modules.mobility.traci.TraCIBaseTrafficManager;

simple PlatoonsTrafficManager like TraCIBaseTrafficManager {

    parameters:
        //platoon insertion time
        double platoonInsertTime @unit("s");
        //platoon insertion speed
        double platoonInsertSpeed @unit("kmph");
        //how many vehicles to inject
        int nCars;
        //platoon size
        int platoonSize;
        //number of lanes
        int nLanes;
        //sumo vehicle type for platooning cars
        string platooningVType;
        //insert distance and headway. distance is computed as:
        //dist = insertHeadway * insertSpeed + insertDistance
        double platoonInsertDistance @unit("m") = default(5m);
        double platoonInsertHeadway @unit("s") = default(0s);
        double platoonLeaderHeadway @unit("s") = default(1.2s);
        @class(PlatoonsTrafficManager);
}
