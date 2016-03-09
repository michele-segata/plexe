//
// Copyright (c) 2012-2016 Michele Segata <segata@ccs-labs.org>
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

#ifndef BASEPOSITIONHELPER_H_
#define BASEPOSITIONHELPER_H_

#include <string>
#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#define INVALID_PLATOON_ID -99

class BasePositionHelper : public BaseApplLayer
{

	public:

		virtual void initialize(int stage);
		virtual void finish();

		/**
		 * Returns the traci external id of this car
		 */
		std::string getExternalId();

		/**
		 * Returns the numeric id of this car
		 */
		virtual int getId();

		/**
		 * Returns the highest id among all platooning cars
		 */
		virtual int getHighestId();

		/**
		 * Returns the position of this vehicle within the platoon
		 */
		virtual int getPosition();

		/**
		 * Returns the id of the i-th vehicle of the own platoon
		 */
		virtual int getMemberId(int position);

		/**
		 * Returns the position of a vehicle of the own platoon
		 */
		virtual int getMemberPosition(int vehicleId);

		/**
		 * Returns the id of the leader of the own platoon
		 */
		virtual int getLeaderId();

		/**
		 * Returns whether this vehicle is the leader of the platoon
		 */
		virtual bool isLeader();

		/**
		 * Returns the id of the vehicle in front of me
		 */
		virtual int getFrontId();

		/**
		 * Returns the id of the platoon
		 */
		virtual int getPlatoonId();

		/**
		 * Returns the lane the platoon is traveling on
		 */
		virtual int getPlatoonLane();

		/**
		 * Returns whether a vehicle is part of my platoon
		 */
		virtual bool isInSamePlatoon(int vehicleId);

		/**
		 * Returns the total number of lanes
		 */
		virtual int getLanesCount();

		/**
		 * Returns the platoon size
		 */
		virtual int getPlatoonSize();

	protected:

		Veins::TraCIMobility* mobility;
		Veins::TraCICommandInterface *traci;
		Veins::TraCICommandInterface::Vehicle *traciVehicle;

		//id of this vehicle
		int myId;
		//number of lanes
		int nLanes;
		//number of cars in the platoon
		int platoonSize;
		//total number of platooning cars in the simulation
		int nCars;
		//largest automated car id in the simulation
		int highestId;

	public:
		BasePositionHelper() {
			mobility = 0;
			traci = 0;
			traciVehicle = 0;
			myId = INVALID_PLATOON_ID;
			nLanes = -1;
			platoonSize = -1;
			nCars = -1;
			highestId = -1;
		}

};

#endif
