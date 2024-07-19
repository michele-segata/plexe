//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package org.car2x.plexe;

import org.car2x.veins.base.modules.IBaseApplLayer;
import org.car2x.veins.modules.mobility.traci.TraCIMobility;
import org.car2x.veins.modules.nic.Nic80211p;

import org.car2x.plexe.utilities.BasePositionHelper;
import org.car2x.plexe.scenarios.BaseScenario;
import org.car2x.plexe.protocols.BaseProtocol;
import org.car2x.plexe.apps.BaseApp;
import org.car2x.plexe.driver.Veins11pRadioDriver;
import org.car2x.plexe.driver.Veins11pRadioServiceDriver;

import org.car2x.plexe.apps.PFApp;
import org.car2x.plexe.maneuver.PFRequester;
import org.car2x.plexe.maneuver.PFAdvertiser;

module CarDouble11pRadio
{
    parameters:
        string scenario_type;
        string helper_type;
        string protocol_type;
        
        @signal[org_car2x_veins_modules_mobility_traciInductionLoopEntry];
        @signal[org_car2x_veins_modules_mobility_traciInductionLoopExit];

        @display("bgb=328,464");
    submodules:

        helper: <helper_type> like BasePositionHelper {
            parameters:
                @display("p=90,100");
        }

        scenario: <scenario_type> like BaseScenario {
            parameters:
                @display("p=90,100");
        }

        appl: PFApp {
            parameters:
                @display("p=60,100");
        }

        pfreq: PFRequester;
        pfadv: PFAdvertiser;
        prot: <protocol_type> like BaseProtocol {
            parameters:
                @display("p=60,200");
        }

        driver11pSafety: Veins11pRadioDriver {
            parameters:
                macpath = "^.nic[0].mac1609_4";
        }

        driver11pService: Veins11pRadioServiceDriver {
            parameters:
                macpath = "^.nic[1].mac1609_4";
        }

        nic[2]: Nic80211p;
        mobility: TraCIMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
    connections allowunconnected:
        driver11pSafety.upperLayerIn <-- prot.radiosOut++;
        driver11pSafety.upperLayerOut --> prot.radiosIn++;
        driver11pService.upperLayerIn <-- prot.radiosOut++;
        driver11pService.upperLayerOut --> prot.radiosIn++;

        nic[0].upperLayerIn <-- driver11pSafety.lowerLayerOut;
        nic[0].upperLayerOut --> driver11pSafety.lowerLayerIn;
        nic[1].upperLayerIn <-- driver11pService.lowerLayerOut;
        nic[1].upperLayerOut --> driver11pService.lowerLayerIn;

        appl.reqIn <-- pfreq.pfout;
        appl.advIn <-- pfadv.pfout;
}
