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

import org.car2x.veins.modules.application.platooning.protocols.BaseProtocol;

simple BBaseProtocol like BaseProtocol
{

    parameters:
        volatile double beaconingInterval @unit(seconds) = default(0.1s);
        //priority (AC) for the messages. 0 = AC_BK, 3 = AC_VO
        int priority;
        //size of platooning messages
        int packetSize;
        int headerLength @unit("bit") = default(0bit);
        @display("i=block/network2");
        @class(BBaseProtocol);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input upperLayerIn[10];
        output upperLayerOut[10];
        input lowerControlIn;
        output lowerControlOut;
        input upperControlIn[10];
        output upperControlOut[10];

}
