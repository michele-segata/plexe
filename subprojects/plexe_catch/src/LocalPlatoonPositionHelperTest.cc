//
// Copyright (C) 2019 Michele Segata <segata@ccs-labs.org>
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

#include "catch2/catch.hpp"

#include "plexe/utilities/LocalPlatoonPositionHelper.h"

using plexe::LocalPlatoonPositionHelper;

SCENARIO("Tests the LocalPlatoonPositionHelper", "[LocalPlatoonPositionHelper]")
{

    GIVEN("An uninitialized LocalPlatoonPositionHelper")
    {
        LocalPlatoonPositionHelper* lpph = nullptr;
        lpph = new LocalPlatoonPositionHelper();
        REQUIRE(lpph != nullptr);

        // independent from platoon formation

        WHEN("The vehicle id was not set")
        {
            THEN("The vehicle id has an invalid value")
            {
                REQUIRE(lpph->getId() == INVALID_VEHICLE_ID);
            }
        }

        WHEN("The vehicle id was set")
        {
            int id = 42;
            lpph->setId(id);
            THEN("The vehicle id has this value")
            {
                REQUIRE(lpph->getId() == id);
            }
        }

        WHEN("The vehicle id was set again")
        {
            int id = 42;
            lpph->setId(id);
            lpph->setId(id + 1);
            THEN("The vehicle id still has the old value")
            {
                REQUIRE(lpph->getId() == id);
            }
        }

        WHEN("The platoon id was not set")
        {
            THEN("The platoon id has an invalid value")
            {
                REQUIRE(lpph->getPlatoonId() == INVALID_PLATOON_ID);
            }
        }

        WHEN("The platoon id was set")
        {
            int id = 42;
            lpph->setPlatoonId(id);
            THEN("The platoon id has this value")
            {
                REQUIRE(lpph->getPlatoonId() == id);
            }
        }

        WHEN("The platoon lane was not set")
        {
            THEN("The platoon lane has an invalid value")
            {
                REQUIRE(lpph->getPlatoonLane() == INVALID_LANE_ID);
            }
        }

        WHEN("The platoon lane was set")
        {
            int lane = 42;
            lpph->setPlatoonLane(lane);
            THEN("The platoon lane has this value")
            {
                REQUIRE(lpph->getPlatoonLane() == lane);
            }
        }

        WHEN("The platoon speed was not set")
        {
            THEN("The platoon speed has an invalid value")
            {
                REQUIRE(lpph->getPlatoonSpeed() == INVALID_SPEED);
            }
        }

        WHEN("The platoon speed was set")
        {
            double speed = 42;
            lpph->setPlatoonSpeed(speed);
            THEN("The platoon speed has this value")
            {
                REQUIRE(lpph->getPlatoonSpeed() == speed);
            }
        }

        // platoon formation was not set (indepenedet from id)

        WHEN("The platoon formation was not set")
        {

            THEN("The platoon has size 0")
            {
                REQUIRE(lpph->getPlatoonSize() == 0);
            }

            THEN("The platoon formation is empty")
            {
                REQUIRE(lpph->getPlatoonFormation().empty());
            }

            THEN("The leader vehicle id has an invalid value")
            {
                REQUIRE(lpph->getLeaderId() == INVALID_VEHICLE_ID);
            }

            THEN("The last vehicle id has an invalid value")
            {
                REQUIRE(lpph->getLastVehicleId() == INVALID_VEHICLE_ID);
            }

            THEN("The id of the member with index 42 has an invalid value")
            {
                REQUIRE(lpph->getMemberId(42) == INVALID_VEHICLE_ID);
            }

            THEN("The position of the member with id 42 has an invalid value")
            {
                REQUIRE(lpph->getMemberPosition(42) == INVALID_POSITION);
            }

            THEN("An arbitrary vehicle (e.g. id 42) is not in the same platoon")
            {
                REQUIRE(!lpph->isInSamePlatoon(42));
            }
        }

        // platoon formation was set (independet from id)

        WHEN("The platoon formation was set")
        {

            std::vector<int> f({0, 1, 2, 3});
            lpph->setPlatoonFormation(f);

            THEN("The platoon has the corresponding size")
            {
                REQUIRE(lpph->getPlatoonSize() == f.size());
            }

            THEN("The platoon formation is not empty")
            {
                REQUIRE(!lpph->getPlatoonFormation().empty());
            }

            THEN("The leader vehicle id is the first value")
            {
                REQUIRE(lpph->getLeaderId() == f.at(0));
            }

            THEN("The last vehicle id has the correct value")
            {
                REQUIRE(lpph->getLastVehicleId() == f.at(3));
            }

            THEN("The id of the member with index 42 has an invalid value")
            {
                REQUIRE(lpph->getMemberId(42) == INVALID_VEHICLE_ID);
            }

            THEN("The position of the member with id 42 has an invalid value")
            {
                REQUIRE(lpph->getMemberPosition(42) == INVALID_POSITION);
            }

            THEN("The id of the member with index 3 has the correct value")
            {
                REQUIRE(lpph->getMemberId(3) == f.at(3));
            }

            THEN("The position of the member with id 3 has the correct value")
            {
                REQUIRE(lpph->getMemberPosition(3) == 3);
            }

            THEN("An arbitrary vehicle (e.g. id 42) is not in the same platoon")
            {
                REQUIRE(!lpph->isInSamePlatoon(42));
            }
        }

        // platoon formation was not set and id was not set (dependent on id)

        WHEN("The vehicle id was not set and the platoon formation was not set")
        {

            THEN("This vehicle is not the leader")
            {
                REQUIRE(!lpph->isLeader());
            }

            THEN("The front vehicle id has an invalid value")
            {
                REQUIRE(lpph->getFrontId() == INVALID_VEHICLE_ID);
            }

            THEN("The back vehicle id has an invalid value")
            {
                REQUIRE(lpph->getBackId() == INVALID_VEHICLE_ID);
            }

            THEN("The vehicle position has an invalid value")
            {
                REQUIRE(lpph->getPosition() == INVALID_POSITION);
            }
        }

        // platoon formation was not set and id was set (dependent on id)

        WHEN("The vehicle id was set and the platoon formation was not set")
        {

            lpph->setId(2);

            THEN("This vehicle is not the leader")
            {
                REQUIRE(!lpph->isLeader());
            }

            THEN("The front vehicle id has an invalid value")
            {
                REQUIRE(lpph->getFrontId() == INVALID_VEHICLE_ID);
            }

            THEN("The back vehicle id has an invalid value")
            {
                REQUIRE(lpph->getBackId() == INVALID_VEHICLE_ID);
            }

            THEN("The vehicle position has an invalid value")
            {
                REQUIRE(lpph->getPosition() == INVALID_POSITION);
            }
        }

        // platoon formation was set and id was not set (dependent on id)

        WHEN("The vehicle id was not set and the platoon formation was set")
        {

            std::vector<int> f({0, 1, 2, 3});
            lpph->setPlatoonFormation(f);

            THEN("This vehicle is not the leader")
            {
                REQUIRE(!lpph->isLeader());
            }

            THEN("The front vehicle id has an invalid value")
            {
                REQUIRE(lpph->getFrontId() == INVALID_VEHICLE_ID);
            }

            THEN("The back vehicle id has an invalid value")
            {
                REQUIRE(lpph->getBackId() == INVALID_VEHICLE_ID);
            }

            THEN("The vehicle position has an invalid value")
            {
                REQUIRE(lpph->getPosition() == INVALID_POSITION);
            }
        }

        // platoon formation was not set and id was set (dependent on id)

        WHEN("The vehicle id was set and the platoon formation was not set")
        {

            lpph->setId(2);

            std::vector<int> f({0, 1, 2, 3});
            lpph->setPlatoonFormation(f);

            THEN("This vehicle is not the leader")
            {
                REQUIRE(!lpph->isLeader());
            }

            THEN("The front vehicle id has the correct value")
            {
                REQUIRE(lpph->getFrontId() == f.at(1));
            }

            THEN("The back vehicle id has the correct value")
            {
                REQUIRE(lpph->getBackId() == f.at(3));
            }

            THEN("The vehicle position has the correct value")
            {
                REQUIRE(lpph->getPosition() == 2);
            }
        }
    }
}
