//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2023 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

simple GeneralPlatooningApp like BaseApp {

parameters:

    // implementation of the join maneuver to be used
    string joinManeuver;
    // implementation of the platoons merge maneuver
    string mergeManeuver;

    double joinSpeedIncrement @unit("kmph") = default(20 kmph);

    bool enableLogging = default(true);
    double loggingInterval @unit(s) = default(0.1 s);

    int headerLength @unit("bit") = default(0 bit);
    @display("i=block/app2");
    @class(plexe::GeneralPlatooningApp);

    //https://doc.omnetpp.org/omnetpp/manual/#sec:simple-modules:available-result-filters-and-recorders
    @signal[nodeId](type="long");
    @statistic[nodeId](source="nodeId"; record=vector);

    @signal[distance](type="double");
    @statistic[distance](source="distance"; record=vector,stats?,timeavg?);

    @signal[relativeSpeed](type="double");
    @statistic[relativeSpeed](source="relativeSpeed"; record=vector,stats?,timeavg?);

    @signal[speed](type="double");
    @statistic[speed](source="speed"; record=vector,stats?,timeavg?);

    @signal[posx](type="double");
    @statistic[posx](source="posx"; record=vector,stats?,timeavg?);

    @signal[posy](type="double");
    @statistic[posy](source="posy"; record=vector,stats?,timeavg?);

    @signal[acceleration](type="double");
    @statistic[acceleration](source="acceleration"; record=vector,stats?,timeavg?);

    @signal[controllerAcceleration](type="double");
    @statistic[controllerAcceleration](source="controllerAcceleration"; record=vector,stats?,timeavg?);

gates:
    input lowerLayerIn;
    output lowerLayerOut;
    input lowerControlIn;
    output lowerControlOut;
}
