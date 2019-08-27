//
// Copyright (c) 2012-2019 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#pragma once

#include "veins/veins.h"

// Version number of last release ("major.minor.patch") or an alpha version, if nonzero
#define PLEXE_VERSION_MAJOR 3
#define PLEXE_VERSION_MINOR 0
#define PLEXE_VERSION_PATCH 0
#define PLEXE_VERSION_ALPHA 1

// Explicitly check Veins version number
#if !(VEINS_VERSION_MAJOR == 5 && VEINS_VERSION_MINOR >= 0)
#error Veins version 5.0 or compatible required
#endif
