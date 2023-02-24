//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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
// Adapted from AlertSender from SimuLTE
//

package org.car2x.plexe.subprojects.plexe_lte;

import inet.applications.contract.IApp;

simple LTECV2XMode3RadioDriver like IApp
{
    parameters:
        @class(plexe::LTECV2XMode3RadioDriver);
        int destinationPort = default(3000);
        string multicastAddress = default("224.0.0.1");
        string interfaceTableModule;

        @display("i=block/ifcard");
    gates:
        output socketOut;
        input socketIn;
        output upperLayerOut;
        input upperLayerIn;
}
