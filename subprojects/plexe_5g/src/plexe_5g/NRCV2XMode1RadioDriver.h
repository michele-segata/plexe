//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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
// Adapted from AlertSender from SimuLTE
//

#pragma once

#include <string.h>
#include <omnetpp.h>
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "plexe/driver/PlexeRadioDriverInterface.h"

namespace plexe {

class NRCV2XMode1RadioDriver : public PlexeRadioDriverInterface, public omnetpp::cSimpleModule {
public:
    ~NRCV2XMode1RadioDriver();
    NRCV2XMode1RadioDriver();

    void setMulticastAddress(std::string address);
    virtual int getDeviceType() override {return PlexeRadioInterfaces::NR_CV2X_MODE1;}

protected:

    inet::UdpSocket socket;

    int destinationPort;
    inet::L3Address multicastAddress;

    int socketInGate;
    int socketOutGate;
    int upperLayerIn;
    int upperLayerOut;

    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(omnetpp::cMessage* msg) override;
};

} // namespace plexe
