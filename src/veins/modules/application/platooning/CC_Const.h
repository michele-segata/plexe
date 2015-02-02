/****************************************************************************/
/// @file    CC_Const.h
/// @author  Michele Segata
/// @date    Fri, 11 Apr 2014
/// @version $Id: $
///
// File defining constants, structs, and enums for cruise controllers
/****************************************************************************/
// Copyright (C) 2012-2014 Michele Segata (segata@ccs-labs.org)
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

namespace Plexe {

/**
 * @brief action that might be requested by the platooning management
 */
enum PLATOONING_LANE_CHANGE_ACTION {
    DRIVER_CHOICE = 0,        //the platooning management is not active, so just let the driver choose the lane
    STAY_IN_CURRENT_LANE = 3, //the car is part of a platoon, so it has to stay on the dedicated platooning lane
    MOVE_TO_FIXED_LANE = 4    //move the car to a specific lane
};

/** @enum ACTIVE_CONTROLLER
 * @brief Determines the currently active controller, i.e., ACC, CACC, or the
 * driver. In future we might need to switch off the automatic controller and
 * leave the control to the mobility model which reproduces a human driver
 */
enum ACTIVE_CONTROLLER
{DRIVER = 0, ACC = 1, CACC = 2, FAKED_CACC = 3, PLOEG = 4};

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

#define MAX_N_CARS 20

/**
 * List of constants defining the type of message in the generic data passing function.
 * The setGenericInformation() method is meant to ease the procedure of adding new data
 * passing to the model, without having to change TraCIConstants, and TraCIServerAPI_Vehicle,
 * as well as a lot of file within Veins.
 */

#define CC_SET_VEHICLE_DATA              0x00    //set data about a vehicle, like position, speed, acceleration, etc
#define CC_SET_VEHICLE_POSITION          0x01    //set the position of the vehicle in the platoon (0 based)
#define CC_SET_PLATOON_SIZE              0x02    //set the number of cars in the platoon
#define CC_GET_VEHICLE_DATA              0x03    //get stored data about a vehicle

//set of controller-related constants
#define CC_SET_CACC_XI                   0x04    //xi
#define CC_SET_CACC_OMEGA_N              0x05    //omega_n
#define CC_SET_CACC_C1                   0x06    //C1
#define CC_SET_ENGINE_TAU                0x07    //engine time constant

#define CC_SET_PLOEG_H                   0x20    //time headway of ploeg's CACC
#define CC_SET_PLOEG_KP                  0x21    //kp parameter of ploeg's CACC
#define CC_SET_PLOEG_KD                  0x22    //kd parameter of ploeg's CACC

}

#endif /* CC_CONST_H */
