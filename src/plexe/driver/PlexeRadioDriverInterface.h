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
};

class PlexeRadioDriverInterface {
public:
    PlexeRadioDriverInterface(){};
    virtual ~PlexeRadioDriverInterface(){};

    // returns the type of the device which is used by the protocols to choose the proper radio interface
    virtual int getDeviceType() = 0;
};

} /* namespace plexe */
