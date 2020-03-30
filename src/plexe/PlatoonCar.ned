//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package org.car2x.plexe;

import org.car2x.veins.base.modules.IBaseApplLayer;
import org.car2x.veins.modules.mobility.traci.TraCIMobility;
import org.car2x.veins.modules.nic.Nic80211p;

import org.car2x.plexe.utilities.BasePositionHelper;
import org.car2x.plexe.scenarios.BaseScenario;
import org.car2x.plexe.protocols.BaseProtocol;
import org.car2x.plexe.apps.BaseApp;
import org.car2x.plexe.driver.Veins11pRadioDriver;

module PlatoonCar
{
    parameters:
        string scenario_type;
        string helper_type;
        string appl_type;
        string protocol_type;

    gates:
        input radioIn; // gate for sendDirect
    submodules:

        helper: <helper_type> like BasePositionHelper {
            parameters:
                @display("p=90,100");
        }

        scenario: <scenario_type> like BaseScenario {
            parameters:
                @display("p=90,100");
        }

        appl: <appl_type> like BaseApp {
            parameters:
                @display("p=60,100");
        }

        prot: <protocol_type> like BaseProtocol {
            parameters:
                @display("p=60,200");
        }

        veins11pDriver: Veins11pRadioDriver {
            parameters:
                @display("p=60,200");
        }

        nic: Nic80211p {
            parameters:
                @display("p=60,400");
        }

        mobility: TraCIMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
    connections allowunconnected:
        nic.upperLayerIn <-- veins11pDriver.lowerLayerOut;
        nic.upperLayerOut --> veins11pDriver.lowerLayerIn;
        veins11pDriver.upperLayerIn <-- prot.lowerLayerOut;
        veins11pDriver.upperLayerOut --> prot.lowerLayerIn;

        radioIn --> nic.radioIn;

}
