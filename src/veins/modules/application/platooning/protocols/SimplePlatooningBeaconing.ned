//
// Copyright (C) 2012-2016 Michele Segata <segata@ccs-labs.org>
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

package org.car2x.veins.modules.application.platooning.protocols;

import org.car2x.veins.modules.application.platooning.protocols.BaseProtocol;

//
// Define static beaconing for platooning
//
simple SimplePlatooningBeaconing like BaseProtocol
{

    parameters:
        volatile double beaconingInterval @unit(seconds);
        //priority (AC) for the messages. 0 = AC_BK, 3 = AC_VO
        int priority;// = default(0);
        //size of platooning messages
        int packetSize;// = default(25);
        int headerLength @unit("bit") = default(0bit);
        //send actual or controller-computed acceleration
        bool useControllerAcceleration = default(true);
		@display("i=block/network2");
        @class(SimplePlatooningBeaconing);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input upperLayerIn[10];
        output upperLayerOut[10];
        input lowerControlIn;
        output lowerControlOut;
        input upperControlIn;
        output upperControlOut;

}