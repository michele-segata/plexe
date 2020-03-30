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

#ifndef SLOTTEDBEACONING_H_
#define SLOTTEDBEACONING_H_

#include "BaseProtocol.h"

namespace plexe {

class SlottedBeaconing : public BaseProtocol {
protected:
    virtual void handleSelfMsg(cMessage* msg);
    virtual void messageReceived(PlatooningBeacon* pkt, veins::BaseFrame1609_4* unicast);

    // number of the slot where we should send our message
    int slotNumber;
    // time after the message received from the leader at which we should send (i.e., slot time)
    SimTime slotTime;

public:
    SlottedBeaconing();
    virtual ~SlottedBeaconing();

    virtual void initialize(int stage);
};

} // namespace plexe

#endif /* SLOTTEDBEACONING_H_ */
