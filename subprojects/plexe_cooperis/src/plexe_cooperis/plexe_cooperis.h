//
// Copyright (C) 2019-2025 Christoph Sommer <sommer@ccs-labs.org>
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

#include "plexe/plexe.h"

// Version number of last release ("major.minor.patch") or an alpha version, if nonzero
#define PLEXE_COOPERIS_VERSION_MAJOR PLEXE_VERSION_MAJOR
#define PLEXE_COOPERIS_VERSION_MINOR PLEXE_VERSION_MINOR
#define PLEXE_COOPERIS_VERSION_PATCH PLEXE_VERSION_PATCH
#define PLEXE_COOPERIS_VERSION_ALPHA PLEXE_VERSION_ALPHA

// Explicitly check Plexe version number
#if !(PLEXE_VERSION_MAJOR == 3 && PLEXE_VERSION_MINOR >= 0)
#error Plexe version 3.0 or compatible required
#endif

// PLEXE_COOPERIS_API macro. Allows us to use the same .h files for both building a .dll and linking against it
#if defined(PLEXE_COOPERIS_EXPORT)
#define PLEXE_COOPERIS_API OPP_DLLEXPORT
#elif defined(PLEXE_COOPERIS_IMPORT)
#define PLEXE_COOPERIS_API OPP_DLLIMPORT
#else
#define PLEXE_COOPERIS_API
#endif
