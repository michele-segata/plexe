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

import org.car2x.veins.base.modules.IBaseApplLayer;

import org.car2x.plexe.protocols.HumanInterferingProtocol;
import org.car2x.plexe.UnicastProtocol;
import org.car2x.veins.modules.mobility.traci.TraCIMobility;
import org.car2x.veins.modules.nic.Nic80211p;


module HumanCar
{

    gates:
        input radioIn; // gate for sendDirect
    submodules:


        prot: HumanInterferingProtocol {
            parameters:
                @display("p=60,200");
        }

        unicast: UnicastProtocol {
            parameters:
                @display("p=60,300");
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
        unicast.upperControlIn <-- prot.lowerControlOut;
        unicast.upperControlOut --> prot.lowerControlIn;
        unicast.upperLayerIn <-- prot.lowerLayerOut;
        unicast.upperLayerOut --> prot.lowerLayerIn;
        nic.upperLayerIn <-- unicast.lowerLayerOut;
        nic.upperLayerOut --> unicast.lowerLayerIn;

        radioIn --> nic.radioIn;

}
