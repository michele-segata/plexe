//
// Copyright (C) 2008-2025 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
// Copyright (C) 2012-2025 Michele Segata <segata@ccs-labs.org>
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

import org.car2x.plexe.PlexeScenario;
import org.car2x.veins.modules.obstacle.ObstacleControl;
import org.car2x.cooperis.Ris;
import org.car2x.cooperis.RisConnectionManager;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;

network TIntersection extends PlexeScenario
{
    parameters:
        bool useBuildingShadowing;
        bool useRis = default(true);
    submodules:
        obstacles: ObstacleControl if useBuildingShadowing {
            @display("p=240,50");
        }
        obstaclesRis: ObstacleControl {
           @display("p=240,50");
        }
        ris[1]: Ris if useRis {
            @display("i=block/broadcast");
        }
        risConnectionManager: RisConnectionManager {
            parameters:
                @display("p=300,0;i=abstract/multicast");
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=300,0");
        }
}
