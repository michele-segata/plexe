/****************************************************************************/
/// @file    CC_Const.h
/// @author  Michele Segata
/// @date    Fri, 11 Apr 2014
/// @version $Id: $
///
// File defining constants, structs, and enums for cruise controllers
/****************************************************************************/
// Copyright (C) 2012-2016 Michele Segata (segata@ccs-labs.org)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef CC_CONST_H
#define CC_CONST_H

#include <string>
#include <sstream>

namespace Plexe {

/**
 * @brief action that might be requested by the platooning management
 */
enum PLATOONING_LANE_CHANGE_ACTION {
    DRIVER_CHOICE = 0,        //the platooning management is not active, so just let the driver choose the lane
    STAY_IN_CURRENT_LANE = 3, //the car is part of a platoon, so it has to stay on the dedicated platooning lane
    MOVE_TO_FIXED_LANE = 4    //move the car to a specific lane
};

/**
 * @brief TraCI modes for lane changing
 */
#define FIX_LC 0b1000000000
#define DEFAULT_NOTRACI_LC 0b1010101010

/** @enum ACTIVE_CONTROLLER
 * @brief Determines the currently active controller, i.e., ACC, CACC, or the
 * driver. In future we might need to switch off the automatic controller and
 * leave the control to the mobility model which reproduces a human driver
 */
enum ACTIVE_CONTROLLER
{DRIVER = 0, ACC = 1, CACC = 2, FAKED_CACC = 3, PLOEG = 4, CONSENSUS = 5};

/**
 * @brief struct used as header for generic data passing to this model through
 * traci
 */
struct CCDataHeader {
    int type;    //type of message. indicates what comes after the header
    int size;    //size of message. indicates how many bytes comes after the header
};

/**
 * Struct defining data passed about a vehicle
 */
struct VEHICLE_DATA {
    int index;           //position in the platoon (0 = first)
    double speed;        //vehicle speed
    double acceleration; //vehicle acceleration
    double positionX;    //position of the vehicle in the simulation
    double positionY;    //position of the vehicle in the simulation
    double time;         //time at which such information was read from vehicle's sensors
    double length;       //vehicle length
};

static std::string packVehicleData(const struct VEHICLE_DATA &d) {
    std::stringstream pack;
    pack << d.index << ":" << d.speed << ":" << d.acceleration << ":" <<
            d.positionX << ":" << d.positionY << ":" << d.time << ":" <<
            d.length;
    return pack.str();
}
static struct VEHICLE_DATA unpackVehicleData(const std::string &d) {
    struct VEHICLE_DATA v;
    int nParameters = sscanf(d.c_str(), "%d:%lf:%lf:%lf:%lf:%lf:%lf", &v.index,
                             &v.speed, &v.acceleration, &v.positionX,
                             &v.positionY, &v.time, &v.length);

    // if not enough parameter, mark as invalid
    if (nParameters != 7)
        v.index = -1;
    return v;
}

#define MAX_N_CARS 8

#define CC_ENGINE_MODEL_FOLM             0x00    //first order lag model
#define CC_ENGINE_MODEL_REALISTIC        0x01    //the detailed and realistic engine model

//parameter names for engine models
#define FOLM_PAR_TAU                     "tau_s"
#define FOLM_PAR_DT                      "dt_s"

#define ENGINE_PAR_VEHICLE               "vehicle"
#define ENGINE_PAR_XMLFILE               "xmlFile"
#define ENGINE_PAR_DT                    "dt_s"

#define CC_PAR_VEHICLE_DATA              "ccvd"   //data about a vehicle, like position, speed, acceleration, etc
#define CC_PAR_VEHICLE_POSITION          "ccvp"   //position of the vehicle in the platoon (0 based)
#define CC_PAR_PLATOON_SIZE              "ccps"   //number of cars in the platoon

//set of controller-related constants
#define CC_PAR_CACC_XI                   "ccxi"    //xi
#define CC_PAR_CACC_OMEGA_N              "ccon"    //omega_n
#define CC_PAR_CACC_C1                   "ccc1"    //C1
#define CC_PAR_ENGINE_TAU                "cctau"   //engine time constant

#define CC_PAR_PLOEG_H                   "ccph"    //time headway of ploeg's CACC
#define CC_PAR_PLOEG_KP                  "ccpkp"   //kp parameter of ploeg's CACC
#define CC_PAR_PLOEG_KD                  "ccpkd"   //kd parameter of ploeg's CACC

#define CC_PAR_VEHICLE_ENGINE_MODEL      "ccem"    //set the engine model for a vehicle

#define CC_PAR_VEHICLE_MODEL             "ccvm"    //set the vehicle model, i.e., engine characteristics
#define CC_PAR_VEHICLES_FILE             "ccvf"    //set the location of the vehicle parameters file

}

#endif /* CC_CONST_H */
