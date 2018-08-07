//
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

#ifndef LOCALPLATOONPOSITIONHELPER_H_
#define LOCALPLATOONPOSITIONHELPER_H_

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"

#define INVALID_VEHICLE_ID -99
#define INVALID_POSITION -1
#define INVALID_LANE_ID -1
#define INVALID_SPEED -1

/**
 * This class stores information about the platoon a vehicle is in. This
 * information includes the formation of the platoon including as well as
 * generel paramter of the platoon (e.g. speed).
 *
 * @note: In the future, this should be independent from the BasePositionHelper
 * and directly integrated in the application. For consistency reasons however,
 * it sub-classes the BasePositionHelper and sticks to its naming-scheme for
 * now.
 *
 * Some methods from the BasePositionHelper are useless or have an undefined
 * behavior within the scope of this class. Therefore, those methods are
 * overridden to throw a cRuntimeError on invocation.
 *
 * @see BasePositionHelper
 * @see cRuntimeError
 */
class LocalPlatoonPositionHelper : public BasePositionHelper {

protected:
    /** Stores the IDs of vehicles currently in the platoon.
     * The values' order corresponds to that of the platoon.
     */
    std::vector<int> formation;

    static int getIdFromExternalId(const std::string externalId);

public:
    /** c'tor of LocalPlatoonPositionHelper */
    LocalPlatoonPositionHelper()
    {
    }

    /** d'tor of LocalPositionHelper */
    virtual ~LocalPlatoonPositionHelper()
    {
    }

    /** Override from BasePositionHelper */
    virtual void initialize(int stage) override;

    /**
     * Return the numeric id of this vehicle
     *
     * @return the id of this vehicle
     */
    virtual int getId() const override
    {
        return myId;
    }

    /**
     * Set the id of this vehicle
     *
     * @param id the new id for this vehicle
     */
    virtual void setId(int id) override
    {
        myId = id;
    }

    /**
     * Return the id of the platoon
     *
     * @return the id of this vehicle's platoon
     */
    virtual int getPlatoonId() const override
    {
        return platoonId;
    }

    /**
     * Set the id of the platoon
     *
     * @param id the id for this vehicle's platoon
     */
    virtual void setPlatoonId(int id) override
    {
        platoonId = id;
    }

    /**
     * Return the id of the leader of the own platoon
     *
     * @return the id of this vehicle's platoon's leader
     */
    virtual int getLeaderId() const override
    {
        if (getPlatoonSize() > 0) {
            return formation.at(0);
        }
        else {
            return INVALID_VEHICLE_ID;
        }
    }

    /**
     * Return whether this vehicle is the leader of the platoon
     *
     * @return true, if this vehicle if the leader, else false
     */
    virtual bool isLeader() const override
    {
        return (getId() != INVALID_VEHICLE_ID && getLeaderId() == getId());
    }

    /**
     * Return the id of the vehicle in front
     *
     * @return the id of the front vehicle
     */
    virtual int getFrontId() const override
    {
        if (isLeader()) {
            return INVALID_VEHICLE_ID;
        }
        else if (getPlatoonSize() < 1) {
            return INVALID_VEHICLE_ID;
        }
        else if (getPosition() == INVALID_POSITION) {
            return INVALID_VEHICLE_ID;
        }
        else {
            return formation.at(getPosition() - 1);
        }
    }

    /**
     * Retun the id of the vehicle in the back
     *
     * @return the if of the vehicle in the back
     */
    virtual int getBackId() const override
    {
        if (getId() == getLastVehicleId()) {
            return INVALID_VEHICLE_ID;
        }
        else if (getPlatoonSize() == 0) {
            return INVALID_VEHICLE_ID;
        }
        else if (getPosition() == INVALID_POSITION) {
            return INVALID_VEHICLE_ID;
        }
        else {
            return formation.at(getPosition() + 1);
        }
    }

    /**
     * Return the position of this vehicle within the platoon
     *
     * @return the index of this vehicle in the platoon formation
     */
    virtual int getPosition() const override
    {
        return getMemberPosition(getId());
    }

    // helper functions

    /**
     * Return whether a vehicle is part of my platoon
     *
     * @param vehicleId the id of the vehicle to check
     * @return true, if the given vehicle is in the same platoon, else false
     */
    virtual bool isInSamePlatoon(int vehicleId) const override
    {
        return (std::find(formation.begin(), formation.end(), vehicleId) != formation.end());
    }

    /**
     * Return the platoon size
     *
     * @return the size
     */
    virtual int getPlatoonSize() const override
    {
        return formation.size();
    }

    /**
     * Return the platoon formation
     *
     * @return the platoon formation
     */
    virtual const std::vector<int>& getPlatoonFormation() const override
    {
        return formation;
    }

    /**
     * Update the platoon formation
     *
     * @param f the new platoon formation
     */
    virtual void setPlatoonFormation(const std::vector<int>& f) override
    {
        formation = f;
    }

    /**
     * Return the id of the last vehicle in the platoon
     *
     * @return the id of the last vehicle
     */
    virtual int getLastVehicleId() const
    {
        if (getPlatoonSize() > 0) {
            return formation.at(getPlatoonSize() - 1);
        }
        else {
            return INVALID_VEHICLE_ID;
        }
    }

    /**
     * Return the id of the i-th vehicle of the own platoon
     *
     * @param position the position of the member vehicle
     * @return the id of the member vehicle
     */
    virtual int getMemberId(int position) const override
    {
        if (getPlatoonSize() < position) {
            return INVALID_VEHICLE_ID;
        }
        else {
            return formation.at(position);
        }
    }

    /**
     * Return the position of the vehicle with the given id within the own
     * platoon
     *
     * @param int vehicleId the id of the member vehicle
     * @return the position of the member vehicle
     */
    virtual int getMemberPosition(int vehicleId) const override
    {
        const auto it = std::find(formation.begin(), formation.end(), vehicleId);
        if (it != formation.end()) {
            return std::distance(formation.begin(), it);
        }
        else {
            return INVALID_POSITION;
        }
    }

    /** Override from BasePositionHelper */
    virtual void setLeaderId(int id) override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual void setFrontId(int id) override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual void setBackId(int id) override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual void setPosition(int position) override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual void setMemberId(int position, int vehicleId) override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual void setMemberPosition(int vehicleId, int position) override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual int getHighestId() const override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual void setHighestId(int id) override
    {
        throw cRuntimeError("Not implemented");
    }

    /** Override from BasePositionHelper */
    virtual void setIsInSamePlatoon(int vehicleId, bool inSamePlatoon) override
    {
        throw cRuntimeError("Not implemented");
    }
};

#endif // LOCALPLATOONPOSITIONHELPER_H_
