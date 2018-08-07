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

#ifndef DYNAMICPOSITIONHELPER_H_
#define DYNAMICPOSITIONHELPER_H_

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"
#include "veins/modules/application/platooning/utilities/DynamicPositionManager.h"

/**
 * Implements a position helper where vehicles can be dynamically added and removed
 */
class DynamicPositionHelper : public BasePositionHelper {

public:
    virtual void initialize(int stage) override;

    /**
     * Returns the position of this vehicle within the platoon
     */
    virtual int getPosition() const override;

    /**
     * Returns the id of the i-th vehicle of the own platoon
     */
    virtual int getMemberId(int position) const override;

    /**
     * Returns the position of a vehicle of the own platoon
     */
    virtual int getMemberPosition(int vehicleId) const override;

    /**
     * Returns the id of the leader of the own platoon
     */
    virtual int getLeaderId() const override;

    /**
     * Returns whether this vehicle is the leader of the platoon
     */
    virtual bool isLeader() const override;

    /**
     * Returns the id of the vehicle in front of me
     */
    virtual int getFrontId() const override;

    /**
     * Returns the id of the platoon
     */
    virtual int getPlatoonId() const override;

    /**
     * Returns the lane the platoon is traveling on
     */
    virtual int getPlatoonLane() const override;

    /**
     * Returns whether a vehicle is part of my platoon
     */
    virtual bool isInSamePlatoon(int vehicleId) const override;

    /**
     * Returns the platoon size
     */
    virtual int getPlatoonSize() const override;

    /**
     * Returns the platoon formation
     */
    virtual const std::vector<int>& getPlatoonFormation() const override;

    /**
     * Updates the platoon formation
     */
    virtual void setPlatoonFormation(const std::vector<int>& formation) override;

public:
    void addVehicleToPlatoon(int vehicleId, int position, int platoonId);

    void removeVehicleFromPlatoon(int vehicleId, int position, int platoonId);

    static int getIdFromExternalId(const std::string externalId);

public:
    DynamicPositionHelper()
        : BasePositionHelper()
        , positions(DynamicPositionManager::getInstance())
    {
    }

protected:
    DynamicPositionManager& positions;
    mutable std::vector<int> formationCache;
};

#endif
