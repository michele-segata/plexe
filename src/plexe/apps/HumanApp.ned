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

simple HumanApp
{
    parameters:

        double enableLaneChangeAfter @unit(s) = default(5s);
        bool enableLogging = default(true);
        double loggingInterval @unit(s) = default(0.1 s);

        @signal[nocommNodeId](type="long");
        @statistic[nodeId](source="nocommNodeId"; record=vector);

        @signal[nocommSpeed](type="double");
        @statistic[speed](source="nocommSpeed"; record=vector,stats?,timeavg?);

        @signal[nocommPosx](type="double");
        @statistic[posx](source="nocommPosx"; record=vector,stats?,timeavg?);

        @signal[nocommPosy](type="double");
        @statistic[posy](source="nocommPosy"; record=vector,stats?,timeavg?);

        @display("i=block/app2");
        @class(plexe::HumanApp);
}
