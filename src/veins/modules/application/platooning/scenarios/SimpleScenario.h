//
// Copyright (C) 2018 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#ifndef SIMPLESCENARIO_H_
#define SIMPLESCENARIO_H_

#include "veins/modules/application/platooning/scenarios/BaseScenario.h"
#include "veins/modules/application/platooning/apps/BaseApp.h"

class SimpleScenario : public BaseScenario {
public:
    virtual void initialize(int stage);

protected:
    // leader average speed
    double leaderSpeed;
    // application layer, used to stop the simulation
    BaseApp* appl;

public:
    SimpleScenario()
        : leaderSpeed(0)
        , appl(nullptr){};
};

#endif
