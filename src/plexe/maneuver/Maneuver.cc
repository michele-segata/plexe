//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#include "plexe/maneuver/Maneuver.h"
#include "plexe/apps/GeneralPlatooningApp.h"

namespace plexe {

Maneuver::Maneuver(GeneralPlatooningApp* app)
    : app(app)
    , positionHelper(app->getPositionHelper())
    , mobility(app->getMobility())
    , traci(app->getTraci())
    , traciVehicle(app->getTraciVehicle())
    , plexeTraci(app->getPlexeTraci())
    , plexeTraciVehicle(app->getPlexeTraciVehicle())
{
}

} // namespace plexe
