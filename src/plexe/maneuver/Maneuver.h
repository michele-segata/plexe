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

#ifndef MANEUVER_H_
#define MANEUVER_H_

#include "plexe/utilities/BasePositionHelper.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "plexe/mobility/CommandInterface.h"
#include "plexe/messages/ManeuverMessage_m.h"
#include "plexe/messages/PlatooningBeacon_m.h"
#include "plexe/messages/UpdatePlatoonFormation_m.h"

namespace plexe {

class GeneralPlatooningApp;

class Maneuver {

public:
    /**
     * Constructor
     *
     * @param app pointer to the generic application used to fetch parameters and inform it about a concluded maneuver
     */
    Maneuver(GeneralPlatooningApp* app);
    virtual ~Maneuver(){};

    /**
     * This method is invoked by the generic application to start the maneuver
     *
     * @param parameters parameters passed to the maneuver
     */
    virtual void startManeuver(const void* parameters) = 0;
    /**
     * This method is invoked by the generic application to abort the maneuver
     */
    virtual void abortManeuver() = 0;

    /**
     * This method is invoked by the generic application when a maneuver message is received.
     * The maneuver must not free the memory of the message, as this might be needed by other maneuvers as well.
     */
    virtual void onManeuverMessage(const ManeuverMessage* mm) = 0;

    /**
     * This method is invoked by the generic application when a beacon message is received
     * The maneuver must not free the memory of the message, as this might be needed by other maneuvers as well.
     */
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) = 0;

protected:
    GeneralPlatooningApp* app;
    BasePositionHelper* positionHelper;
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    traci::CommandInterface::Vehicle* plexeTraciVehicle;
};

} // namespace plexe

#endif
