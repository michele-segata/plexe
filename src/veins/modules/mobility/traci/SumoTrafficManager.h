//
// Copyright (C) 2013-2018 Michele Segata <segata@ccs-labs.org>, Stefan Joerer <joerer@ccs-labs.org>
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

#ifndef SUMOTRAFFICMANAGER_H_
#define SUMOTRAFFICMANAGER_H_

#include "veins/modules/mobility/traci/TraCIBaseTrafficManager.h"

/**
 * This traffic manager, set by default, does nothing. The idea
 * is that, by default, traffic flows are managed by SUMO itself,
 * as defined in the rou.xml file
 */
class SumoTrafficManager : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage);
    virtual void finish();

public:
    SumoTrafficManager()
    {
    }
};

#endif /* SUMOTRAFFICMANAGER_H_ */
