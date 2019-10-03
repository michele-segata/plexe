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

package org.car2x.plexe.protocols;

import org.car2x.veins.base.modules.IBaseApplLayer;

//
// Define the base protocol for platooning. every improved
// protocol must derive from this
//
moduleinterface BaseProtocol extends IBaseApplLayer
{

    parameters:
        volatile double beaconingInterval @unit(seconds);// = default(0.1 seconds);
        //priority (AC) for the messages. 0 = AC_BK, 3 = AC_VO
        int priority;// = default(0);
        //size of platooning messages
        int packetSize;// = default(25);
        @display("i=block/network2");
        @class(plexe::BaseProtocol);
     gates:
         input upperLayerIn[10];
         output upperLayerOut[10];
         input upperControlIn[10];
         output upperControlOut[10];
}
