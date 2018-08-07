//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
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

#ifndef JOINMANEUVERSCENARIO_H_
#define JOINMANEUVERSCENARIO_H_

#include "veins/modules/application/platooning/scenarios/BaseScenario.h"
#include "veins/modules/application/platooning/apps/GeneralPlatooningApp.h"

#include "veins/modules/application/platooning/messages/ManeuverMessage_m.h"

class JoinManeuverScenario : public BaseScenario {

protected:
    // message used to start the maneuver
    cMessage* startManeuver;
    // pointer to protocol
    GeneralPlatooningApp* app;

public:
    static const int MANEUVER_TYPE = 12347;

    virtual void initialize(int stage) override;

protected:
    void sendUnicast(cPacket* msg, int destination);

private:
public:
    JoinManeuverScenario()
    {
        startManeuver = nullptr;
        app = nullptr;
    }
    virtual ~JoinManeuverScenario();

protected:
    virtual void handleSelfMsg(cMessage* msg) override;

    void prepareManeuverCars(int platoonLane);
    void setupFormation();
};

#endif
