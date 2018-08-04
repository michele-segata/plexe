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

#include "veins/modules/application/platooning/utilities/JoinPositionHelper.h"

Define_Module(JoinPositionHelper);

void JoinPositionHelper::initialize(int stage)
{

    BasePositionHelper::initialize(stage);

    if (stage == 0) {
        myId = getIdFromExternalId(getExternalId());
    }
}

bool JoinPositionHelper::isInSamePlatoon(const int vehicleId) const
{
    return true;
}

int JoinPositionHelper::getIdFromExternalId(const std::string externalId)
{
    int dotIndex = externalId.find_last_of('.');
    std::string strId = externalId.substr(dotIndex + 1);
    return strtol(strId.c_str(), 0, 10);
}
