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

package org.car2x.plexe.subprojects.plexe_vlc;

import org.car2x.veins.base.modules.IBaseApplLayer;
import org.car2x.veins.modules.mobility.traci.TraCIMobility;

import org.car2x.plexe.utilities.BasePositionHelper;
import org.car2x.plexe.scenarios.BaseScenario;
import org.car2x.plexe.protocols.BaseProtocol;
import org.car2x.plexe.apps.BaseApp;

import org.car2x.plexe.subprojects.plexe_vlc.VeinsVLCRadioDriver;

import org.car2x.veinsvlc.NicVlc;
import org.car2x.veinsvlc.Splitter;

module PlatoonCarVlc
{
    parameters:
        string scenario_type;
        string helper_type;
        string appl_type;
        string protocol_type;

        @display("bgb=358,382");
    gates:
        input headLightIn;
        input tailLightIn;
    submodules:

        helper: <helper_type> like BasePositionHelper {
            parameters:
                @display("p=29,100");
        }

        scenario: <scenario_type> like BaseScenario {
            parameters:
                @display("p=29,36");
        }

        appl: <appl_type> like BaseApp {
            parameters:
                @display("p=165,52");
        }

        prot: <protocol_type> like BaseProtocol {
            parameters:
                @display("p=165,120");
        }

        veinsVlcDriver: VeinsVLCRadioDriver {
            parameters:
                @display("p=165,184");
        }

        nicVlcHead: NicVlc {
            parameters:
                @display("p=165,337");
        }

        nicVlcTail: NicVlc {
            parameters:
                @display("p=255,337");
        }

        splitter: Splitter {
            @display("p=165,254");
        }

        veinsmobility: TraCIMobility {
            parameters:
                @display("p=29,167;i=block/cogwheel");
        }
    connections allowunconnected:

        veinsVlcDriver.upperLayerIn <-- prot.radiosOut++;
        veinsVlcDriver.upperLayerOut --> prot.radiosIn++;

        splitter.applicationIn <-- veinsVlcDriver.lowerLayerOut;
        splitter.applicationOut --> veinsVlcDriver.lowerLayerIn;

        splitter.nicVlcHeadOut --> nicVlcHead.upperLayerIn;
        splitter.nicVlcHeadIn <-- nicVlcHead.upperLayerOut;

        splitter.nicVlcTailOut --> nicVlcTail.upperLayerIn;
        splitter.nicVlcTailIn <-- nicVlcTail.upperLayerOut;

        headLightIn --> nicVlcHead.radioIn;
        tailLightIn --> nicVlcTail.radioIn;

}
