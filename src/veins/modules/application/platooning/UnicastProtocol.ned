//
// Copyright (C) 2012-2018 Michele Segata <segata@ccs-labs.org>
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

package org.car2x.veins.modules.application.platooning;

import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

//
// This simple module implements a basic unicast (acknowledged)
// communication protocol for the platooning application. The
// standard Mac1609.4 supports only broadcast transmissions.
// For the platooning application, however, a reliable (acknowledged)
// unicast communication is needed.
// The module has a queue of messages coming from the upper
// layer. The first packet in the queue is sent out and the
// protocol waits for the ack. If the ack comes back, the
// packet is removed from the queue and the next in the queue
// (if any) is sent. Otherwise the protocol tries to send it
// again until it is acknowledged. If the transmission fails
// after a certain number of attempts, an error is communicated
// to the upper layer.
//
simple UnicastProtocol extends BaseWaveApplLayer
{
    parameters:
        //maximum queue size. set to 0 for infinite queue
        int queueSize = default(0);
        //maximum number of attempts
        int maxAttempts = default(16);
        //ack timeout
        double ackTimeout @unit(s) = default(0.001s);
        //packet loss rate (between 0 and 1)
        double packetLossRate = default(0);

        @class(UnicastProtocol);
        @display("i=msg/mail");

        @signal[droppedExceededAttempts](type=bool);
        @statistic[droppedExceededAttempts](title="DroppedExceededAttempts"; source=constant1(droppedExceededAttempts); record=count,vector?);
        @signal[transmissionAttempts](type=long);
        @statistic[transmissionAttempts](title="TransmissionAttempts"; source=transmissionAttempts; record=mean,vector);

    gates:
        //control input/output from/to the upper layer. can be
        //used for example to set the mac layer address
        //or to notify the application of a transmission
        //error
        input upperControlIn;
        output upperControlOut;
        //input/output from/to the upper layer application
        input upperLayerIn;
        output upperLayerOut;
        //input/output from/to the lower layer (Mac 1609.4)
        //is already defined by BaseWaveApplLayer

}
