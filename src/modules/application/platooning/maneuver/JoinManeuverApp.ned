//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package org.mixim.modules.application.platooning.maneuver;

import org.mixim.modules.application.platooning.apps.BaseApp;

simple JoinManeuverApp like BaseApp
{
    parameters:
        int headerLength @unit("bit") = default(0 bit);
        //number of lanes in the higway
        int nLanes = default(4);
        //send actual or controller-computed acceleration
        bool useControllerAcceleration = default(true);
        //cacc and engine related parameters
        double caccXi;
        double caccOmegaN @unit("Hz");
        double caccC1;
        double engineTau @unit("s");

        @display("i=block/app2");
        @class(JoinManeuverApp);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}
