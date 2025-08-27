//
// Copyright (C) 2020-2023 Michele Segata <segata@ccs-labs.org>
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

#pragma once

#include <string>

namespace plexe {

enum PlexeRadioInterfaces {
    // up to 16 interfaces
    ALL = 65535,
    VEINS_11P = 1,
    LTE_CV2X_MODE3 = 2,
    VEINS_VLC_FRONT = 4,
    VEINS_VLC_BACK = 8,
    COOPERIS = 16,
    NR_CV2X_MODE1 = 32,
};

class PlexeRadioDriverInterface {
public:
    PlexeRadioDriverInterface(){};
    virtual ~PlexeRadioDriverInterface(){};

    // returns the type of the device which is used by the protocols to choose the proper radio interface
    virtual int getDeviceType() = 0;

    static enum PlexeRadioInterfaces parseRadioInterface(const char* radioInterface)
    {
        if (!strcmp(radioInterface, "ALL"))
            return ALL;
        else if (!strcmp(radioInterface, "VEINS_11P"))
            return VEINS_11P;
        else if (!strcmp(radioInterface, "LTE_CV2X_MODE3"))
            return LTE_CV2X_MODE3;
        else if (!strcmp(radioInterface, "VEINS_VLC_FRONT"))
            return VEINS_VLC_FRONT;
        else if (!strcmp(radioInterface, "VEINS_VLC_BACK"))
            return VEINS_VLC_BACK;
        else if (!strcmp(radioInterface, "COOPERIS"))
            return COOPERIS;
        else if (!strcmp(radioInterface, "NR_CV2X_MODE1"))
            return NR_CV2X_MODE1;
        else
            throw omnetpp::cRuntimeError("Unknown radio interface: '%s'", radioInterface);
    }

    static std::string radioInterfacesToString(enum PlexeRadioInterfaces interfaces)
    {
        bool first = true;
        std::stringstream ss;
        if (interfaces & PlexeRadioInterfaces::VEINS_11P) {
            ss << "VEINS_11P";
            first = false;
        }
        if (interfaces & PlexeRadioInterfaces::LTE_CV2X_MODE3) {
            if (!first)
                ss << " | ";
            ss << "LTE_CV2X_MODE3";
            first = false;
        }
        if (interfaces & PlexeRadioInterfaces::VEINS_VLC_FRONT) {
            if (!first)
                ss << " | ";
            ss << "VEINS_VLC_FRONT";
            first = false;
        }
        if (interfaces & PlexeRadioInterfaces::VEINS_VLC_BACK) {
            if (!first)
                ss << " | ";
            ss << "VEINS_VLC_BACK";
            first = false;
        }
        if (interfaces & PlexeRadioInterfaces::COOPERIS) {
            if (!first)
                ss << " | ";
            ss << "COOPERIS";
            first = false;
        }
        if (interfaces & PlexeRadioInterfaces::NR_CV2X_MODE1) {
            if (!first)
                ss << " | ";
            ss << "NR_CV2X_MODE1";
            first = false;
        }
        return ss.str();
    }
};

} /* namespace plexe */
