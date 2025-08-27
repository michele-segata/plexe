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

package org.car2x.plexe.apps;

import org.car2x.plexe.apps.BaseApp;

simple HelloPlexeApp like BaseApp
{
    parameters:
        int headerLength @unit("bit") = default(0 bit);
        @display("i=block/app2");
        @class(plexe::HelloPlexeApp);

        // interface to be used to send the message.
        // this uses the names of the enum defined in PlexeRadioDriverInterface.h, and it is here as an example
        // NOTICE THAT:
        // 1) if you want to use a special interface, say COOPERIS, you will need to compile the CoopeRIS framework as well
        //    and use a car module that has such an interface
        // 2) as the enum values are strings you cannot use the binary OR operator to choose multiple interfaces.
        //    With the enum in C++ you can do "VEINS_11P | COOPERIS" to send a frame via those two interfaces. Here you
        //    cannot as this is just meant as an example
        string interfaceToUse @enum("ALL", "VEINS_11P", "LTE_CV2X_MODE3", "VEINS_VLC_FRONT", "VEINS_VLC_BACK", "COOPERIS");
        // send the message X seconds after the initialization of the module
        double sendMessageAfter @unit(s) = default(3s);
        // id of the vehicle that should send the message
        int senderVehicleId = default(0);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}
