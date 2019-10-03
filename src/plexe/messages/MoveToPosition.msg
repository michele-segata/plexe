//
// Copyright (C) 2012-2019 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2019 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

cplusplus{{
#include "ManeuverMessage_m.h"
}};

packet ManeuverMessage;

// Request to move the correct position.
// Is sent from the leader of the Platoon to an accepted joiner.
// Needs a successful JoinResponse to be sent be before.
// Contains information about the Platoon and the position to join.
packet MoveToPosition extends ManeuverMessage {
    double platoonSpeed;
    int platoonLane;
    int newPlatoonFormation[];
}
