//
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

#ifndef SIMPLESCENARIO_H_
#define SIMPLESCENARIO_H_

#include "plexe/scenarios/BaseScenario.h"
#include "plexe/apps/BaseApp.h"

namespace plexe {

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

} // namespace plexe

#endif
