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

package org.car2x.plexe.scenarios;

import org.car2x.plexe.scenarios.BBaseScenario;

simple AccelerateAndBrakeScenario extends BBaseScenario
{
    parameters:
        //braking deceleration
        double brakingDeceleration @unit("mpsps") = default(4mpsps);
        //acceleration deceleration
        double acceleration @unit("mpsps") = default(4mpsps);
        //start accelerating at time
        double startAccelerating @unit("s") = default(5s);
        //start braking at time
        double startBraking @unit("s") = default(30s);

        @display("i=block/app2");
        @class(plexe::AccelerateAndBrakeScenario);
}
