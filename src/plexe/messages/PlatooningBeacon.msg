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

packet PlatooningBeacon {
    //id of the originator
    int vehicleId = 0;
    double controllerAcceleration = 0;
    double acceleration = 0;
    double speed = 0;
    double positionX = 0;
    double positionY = 0;
    double time = 0;
    int sequenceNumber = 0;
    double length = 0;
    double speedX = 0;
    double speedY = 0;
    double angle = 0;
}
