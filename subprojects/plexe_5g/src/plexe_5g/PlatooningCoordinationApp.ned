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

package org.car2x.plexe.subprojects.plexe_5g;

import inet.applications.contract.IApp;

simple PlatooningCoordinationApp like IApp
{
    parameters:
        @class(plexe::PlatooningCoordinationApp);
        // indicates how frequently update messages should be sent to the traffic authority
        double sendUpdateInterval @unit(s) = default(1s);
        // indicates when the platoon should send a message for searching for other platoons
        double sendPlatoonSearchTime @unit(s) = default(2s);
        string localAddress = default(""); // local address or empty ("")
        int localPort; // local port number
        string connectAddress;  // server address (may be symbolic)
        int connectPort = default(3000); // port number to connect to 
        string dataTransferMode = default("object");
        string interfaceTableModule;
        int timeToLive = default(-1);
        int dscp = default(-1);
        int tos = default(-1);

        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[connect](type=long);  // 1 for open, -1 for close
        @display("i=block/source");
    gates:
        input socketIn;
        output socketOut;
}
