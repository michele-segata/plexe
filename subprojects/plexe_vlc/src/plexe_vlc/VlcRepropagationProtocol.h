//
// Copyright (C) 2012-2020 Michele Segata <segata@ccs-labs.org>
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

#include "plexe/protocols/BaseProtocol.h"

namespace plexe {

class VlcRepropagationProtocol : public BaseProtocol {
private:
    typedef std::map<int, int> SeqNumbers;
    SeqNumbers seqNumbers;

    /**
     * Updates the list of known packets and tells whether the packet needs to be repropagated
     */
    bool updateAndCheckRepropagation(PlatooningBeacon* pkt);
protected:
    virtual void handleSelfMsg(cMessage* msg);
    virtual void messageReceived(PlatooningBeacon* pkt, veins::BaseFrame1609_4* frame);

public:
    VlcRepropagationProtocol();
    virtual ~VlcRepropagationProtocol();

    virtual void initialize(int stage);
};

} // namespace plexe
