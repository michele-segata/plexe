//
// Copyright (C) 2012-2019 Michele Segata <segata@ccs-labs.org>
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

#ifndef CC_CONST_H
#define CC_CONST_H

#include <string>
#include <sstream>

namespace {
const std::string parameter_prefix = "carFollowModel.";
} // namespace

namespace plexe {

/**
 * @brief action that might be requested by the platooning management
 */
enum PLATOONING_LANE_CHANGE_ACTION {
    DRIVER_CHOICE = 0, // the platooning management is not active, so just let the driver choose the lane
    STAY_IN_CURRENT_LANE = 3, // the car is part of a platoon, so it has to stay on the dedicated platooning lane
    MOVE_TO_FIXED_LANE = 4 // move the car to a specific lane
};

/**
 * @brief TraCI modes for lane changing
 */
#define FIX_LC 0b1000000000
#define FIX_LC_AGGRESSIVE 0b0000000000
#define DEFAULT_NOTRACI_LC 0b1010101010

/** @enum ACTIVE_CONTROLLER
 * @brief Determines the currently active controller, i.e., ACC, CACC, or the
 * driver. In future we might need to switch off the automatic controller and
 * leave the control to the mobility model which reproduces a human driver
 */
enum ACTIVE_CONTROLLER {
    DRIVER = 0,
    ACC = 1,
    CACC = 2,
    FAKED_CACC = 3,
    PLOEG = 4,
    CONSENSUS = 5,
    FLATBED = 6
};

/**
 * @brief struct used as header for generic data passing to this model through
 * traci
 */
struct CCDataHeader {
    int type; // type of message. indicates what comes after the header
    int size; // size of message. indicates how many bytes comes after the header
};

/**
 * Struct defining data passed about a vehicle
 */
struct VEHICLE_DATA {
    int index; // position in the platoon (0 = first)
    double speed; // vehicle speed
    double acceleration; // vehicle acceleration
    double positionX; // position of the vehicle in the simulation
    double positionY; // position of the vehicle in the simulation
    double time; // time at which such information was read from vehicle's sensors
    double length; // vehicle length
    double u; // controller acceleration
    double speedX; // vehicle speed on the X axis
    double speedY; // vehicle speed on the Y axis
    double angle; // vehicle angle in radians
};

#define MAX_N_CARS 8

#define CC_ENGINE_MODEL_FOLM 0x00 // first order lag model
#define CC_ENGINE_MODEL_REALISTIC 0x01 // the detailed and realistic engine model

// parameter names for engine models
#define FOLM_PAR_TAU (parameter_prefix + "tau_s")
#define FOLM_PAR_DT (parameter_prefix + "dt_s")

#define ENGINE_PAR_VEHICLE (parameter_prefix + "vehicle")
#define ENGINE_PAR_XMLFILE (parameter_prefix + "xmlFile")
#define ENGINE_PAR_DT (parameter_prefix + "dt_s")

#define CC_PAR_VEHICLE_DATA (parameter_prefix + "ccvd") // data about a vehicle, like position, speed, acceleration, etc
#define CC_PAR_VEHICLE_POSITION (parameter_prefix + "ccvp") // position of the vehicle in the platoon (0 based)
#define CC_PAR_PLATOON_SIZE (parameter_prefix + "ccps") // number of cars in the platoon

// set of controller-related constants
#define CC_PAR_CACC_XI (parameter_prefix + "ccxi") // xi
#define CC_PAR_CACC_OMEGA_N (parameter_prefix + "ccon") // omega_n
#define CC_PAR_CACC_C1 (parameter_prefix + "ccc1") // C1
#define CC_PAR_ENGINE_TAU (parameter_prefix + "cctau") // engine time constant

#define CC_PAR_UMIN (parameter_prefix + "ccumin") // lower saturation for u
#define CC_PAR_UMAX (parameter_prefix + "ccumax") // upper saturation for u

#define CC_PAR_PLOEG_H (parameter_prefix + "ccph") // time headway of ploeg's CACC
#define CC_PAR_PLOEG_KP (parameter_prefix + "ccpkp") // kp parameter of ploeg's CACC
#define CC_PAR_PLOEG_KD (parameter_prefix + "ccpkd") // kd parameter of ploeg's CACC

#define CC_PAR_FLATBED_KA (parameter_prefix + "ccfka") // ka parameter of flatbed CACC
#define CC_PAR_FLATBED_KV (parameter_prefix + "ccfkv") // kv parameter of flatbed CACC
#define CC_PAR_FLATBED_KP (parameter_prefix + "ccfkp") // kp parameter of flatbed CACC
#define CC_PAR_FLATBED_H (parameter_prefix + "ccfh") // h parameter of flatbed CACC
#define CC_PAR_FLATBED_D (parameter_prefix + "ccfd") // distance parameter of flatbed CACC

#define CC_PAR_VEHICLE_ENGINE_MODEL (parameter_prefix + "ccem") // set the engine model for a vehicle

#define CC_PAR_VEHICLE_MODEL (parameter_prefix + "ccvm") // set the vehicle model, i.e., engine characteristics
#define CC_PAR_VEHICLES_FILE (parameter_prefix + "ccvf") // set the location of the vehicle parameters file

// set CACC constant spacing
#define PAR_CACC_SPACING (parameter_prefix + "ccsp")

// get ACC computed acceleration when faked CACC controller is enabled
#define PAR_ACC_ACCELERATION (parameter_prefix + "ccacc")

// determine whether a vehicle has crashed or not
#define PAR_CRASHED (parameter_prefix + "cccr")

// set a fixed acceleration to a vehicle controlled by CC/ACC/CACC
#define PAR_FIXED_ACCELERATION (parameter_prefix + "ccfa")

// get vehicle speed and acceleration, needed for example by the platoon leader (get: vehicle)
#define PAR_SPEED_AND_ACCELERATION (parameter_prefix + "ccsa")

// set speed and acceleration of the platoon leader
#define PAR_LEADER_SPEED_AND_ACCELERATION (parameter_prefix + "cclsa")

// set whether CACCs should use real or controller acceleration
#define PAR_USE_CONTROLLER_ACCELERATION (parameter_prefix + "ccca")

// get lane count for the street the vehicle is currently traveling
#define PAR_LANES_COUNT (parameter_prefix + "cclc")

// set the cruise control desired speed
#define PAR_CC_DESIRED_SPEED (parameter_prefix + "ccds")

// set the currently active vehicle controller which can be either the driver, or the ACC or the CACC
#define PAR_ACTIVE_CONTROLLER (parameter_prefix + "ccac")

// get whether a cruise controller is installed in the car
#define PAR_CC_INSTALLED (parameter_prefix + "ccci")

// get radar data from the car
#define PAR_RADAR_DATA (parameter_prefix + "ccrd")

// communicate with the cruise control to give him fake indications. this can be useful when you want
// to advance a vehicle to a certain position, for example, for joining a platoon. clearly the ACC
// must always take into consideration both fake and real data
#define PAR_LEADER_FAKE_DATA (parameter_prefix + "cclfd")
#define PAR_FRONT_FAKE_DATA (parameter_prefix + "ccffd")

// get the distance that a car has to travel until it reaches the end of its route
#define PAR_DISTANCE_TO_END (parameter_prefix + "ccdte")

// get the distance from the beginning of the route
#define PAR_DISTANCE_FROM_BEGIN (parameter_prefix + "ccdfb")

// set speed and acceleration of preceding vehicle
#define PAR_PRECEDING_SPEED_AND_ACCELERATION (parameter_prefix + "ccpsa")

// set ACC headway time
#define PAR_ACC_HEADWAY_TIME (parameter_prefix + "ccaht")

// return engine information (for the realistic engine model)
#define PAR_ENGINE_DATA (parameter_prefix + "cced")

// enabling/disabling auto feeding
#define PAR_USE_AUTO_FEEDING (parameter_prefix + "ccaf")

// enabling/disabling data prediction
#define PAR_USE_PREDICTION (parameter_prefix + "ccup")

// add/remove members from own platoon
#define PAR_ADD_MEMBER (parameter_prefix + "ccam")
#define PAR_REMOVE_MEMBER (parameter_prefix + "ccrm")

// let the leader automatically change lane for the whole platoon if there is a speed advantage
#define PAR_ENABLE_AUTO_LANE_CHANGE (parameter_prefix + "ccalc")

} // namespace plexe

#endif /* CC_CONST_H */
