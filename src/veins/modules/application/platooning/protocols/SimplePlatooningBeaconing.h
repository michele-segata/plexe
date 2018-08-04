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

#ifndef SIMPLEPLATOONINGBEACONING_H_
#define SIMPLEPLATOONINGBEACONING_H_

#include "BaseProtocol.h"

class SimplePlatooningBeaconing : public BaseProtocol {
protected:
    virtual void handleSelfMsg(cMessage* msg);
    virtual void messageReceived(PlatooningBeacon* pkt, UnicastMessage* unicast);

public:
    SimplePlatooningBeaconing();
    virtual ~SimplePlatooningBeaconing();

    virtual void initialize(int stage);
};

#endif /* SIMPLEPLATOONINGBEACONING_H_ */
