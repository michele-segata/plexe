//
// Copyright (C) 2013-2019 Michele Segata <segata@ccs-labs.org>, Stefan Joerer <joerer@ccs-labs.org>
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

package org.car2x.plexe.traffic;

import org.car2x.plexe.mobility.TraCIBaseTrafficManager;

simple RingTrafficManager like TraCIBaseTrafficManager {

    parameters:
        //number of vehicles per platoon. can be a random distribution
        //e.g., uniform(3,8)
        volatile int platoonSize = default(8);
        //number of platoons to inject 
        int nPlatoons = default(1);
        //number of lanes in the ring scenario. first platoon is inserted in
        //the first lane, second in the second, ..., and then the procedure
        //starts again from the first lane
        int nLanes;
        //platoon insertion speed. can be a random distribution
        volatile double platoonInsertSpeed @unit("kmph");
        //sumo vehicle type for platooning cars
        string platooningVType;
        //insert distance and headway. distance is computed as:
        //dist = insertHeadway * insertSpeed + insertDistance
        //default values are for PATH's CACC
        double platoonInsertDistance @unit("m") = default(5m);
        double platoonInsertHeadway @unit("s") = default(0s);
        double platoonLeaderHeadway @unit("s") = default(1.2s);
        @class(plexe::RingTrafficManager);
}
