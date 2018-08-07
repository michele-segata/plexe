//
// Copyright (c) 2012-2018 Michele Segata <segata@ccs-labs.org>
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

#ifndef SLOTTEDBEACONING_H_
#define SLOTTEDBEACONING_H_

#include "BaseProtocol.h"

class SlottedBeaconing : public BaseProtocol {
protected:
    virtual void handleSelfMsg(cMessage* msg);
    virtual void messageReceived(PlatooningBeacon* pkt, UnicastMessage* unicast);

    // number of the slot where we should send our message
    int slotNumber;
    // time after the message received from the leader at which we should send (i.e., slot time)
    SimTime slotTime;

public:
    SlottedBeaconing();
    virtual ~SlottedBeaconing();

    virtual void initialize(int stage);
};

#endif /* SLOTTEDBEACONING_H_ */
