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

#ifndef JOINPOSITIONHELPER_H_
#define JOINPOSITIONHELPER_H_

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

class JoinPositionHelper : public BasePositionHelper {

public:
    virtual void initialize(int stage) override;

    virtual bool isInSamePlatoon(const int vehicleId) const override;

public:
    static int getIdFromExternalId(const std::string externalId);

    JoinPositionHelper()
        : BasePositionHelper()
    {
    }
};

#endif
