//
// Copyright (C) 2012-2020 Michele Segata <segata@ccs-labs.org>
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

package org.car2x.plexe.subprojects.plexe_lte;

import inet.applications.contract.ITCPApp;

simple PlatoonTrafficAuthority like ITCPApp
{
    parameters:
        @class(plexe::PlatoonTrafficAuthority);
        int localPort = default(3000); // local port number 
        // distance from the approached platoon at which the TA stops commanding the approaching platoon
        double approachDistanceThreshold @unit(m) = default(200m);
        // speed difference to be used to approach a platoon
        double approachSpeedDelta @unit(mps) = default(10mps);
        string interfaceTableModule;

        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[connect](type=long);  // 1 for open, -1 for close
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}
