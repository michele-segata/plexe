//
// Copyright (C) 2012-2022 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2022 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include <algorithm>

#include "plexe/maneuver/JoinAtBack.h"

using namespace veins;

namespace plexe {

class MergeAtBack : public JoinAtBack {

public:
    /**
     * Constructor
     *
     * @param app pointer to the generic application used to fetch parameters and inform it about a concluded maneuver
     */
    MergeAtBack(GeneralPlatooningApp* app);
    virtual ~MergeAtBack();

    /**
     * This method is invoked by the generic application to start the maneuver
     *
     * @param parameters parameters passed to the maneuver
     */
    virtual void startManeuver(const void* parameters) override;

    /**
     * Handles a JoinPlatoonRequest in the context of this application
     *
     * @param JoinPlatoonRequest msg to handle
     */
    virtual void handleJoinPlatoonRequest(const JoinPlatoonRequest* msg) override;

    /**
     * Handles a MergePlatoonRequest in the context of this application
     *
     * @param MergePlatoonRequest msg to handle
     */
    virtual void handleMergePlatoonRequest(const MergePlatoonRequest* msg) override;

    /**
     * Handles a JoinFormation in the context of this application
     *
     * @param JoinFormation msg to handle
     */
    virtual void handleJoinFormation(const JoinFormation* msg) override;

    /**
     * Handles a JoinFormationAck in the context of this application
     *
     * @param JoinFormationAck msg to handle
     */
    virtual void handleJoinFormationAck(const JoinFormationAck* msg) override;

    virtual bool handleSelfMsg(cMessage* msg) override;

protected:
    // store the old formation this vehicle is leader for to communicate it to the leader of the platoon we are merging with
    std::vector<int> oldFormation;
    // store the old platoon id before changing it
    int oldPlatoonId;

    // message used to periodically check for the distance while performing the final approach
    cMessage* checkDistance;
};

} // namespace plexe
