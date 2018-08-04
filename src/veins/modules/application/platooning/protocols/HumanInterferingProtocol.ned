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

package org.car2x.veins.modules.application.platooning.protocols;

import org.car2x.veins.base.modules.IBaseApplLayer;

//
// Define a protocol that simply sends packets to interfere
//
simple HumanInterferingProtocol like IBaseApplLayer
{

    parameters:
        volatile double beaconingInterval @unit(seconds);
        //priority (AC) for the messages. 0 = AC_BK, 3 = AC_VO
        int priority;
        //size of platooning messages
        int packetSize;
        //tx power in mW
        double txPower @unit("mW");
        //bitrate in bps
        int bitrate @unit("bps");

        int headerLength @unit("bit") = default(0 bit);
        @display("i=block/network2");
        @class(HumanInterferingProtocol);

    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}
