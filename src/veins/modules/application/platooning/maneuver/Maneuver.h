//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
// Copyright (c) 2018 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#ifndef MANEUVER_H_
#define MANEUVER_H_

class GeneralPlatooningApp;

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/modules/application/platooning/messages/ManeuverMessage_m.h"
#include "veins/modules/application/platooning/messages/PlatooningBeacon_m.h"
#include "veins/modules/application/platooning/messages/UpdatePlatoonFormation_m.h"

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
    Veins::TraCIMobility* mobility;
    Veins::TraCICommandInterface* traci;
    Veins::TraCICommandInterface::Vehicle* traciVehicle;
};

#endif
