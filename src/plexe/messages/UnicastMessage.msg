//
// Copyright (C) 2012-2016 Michele Segata <segata@ccs-labs.org>
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

enum UnicastMessageType {
	DATA = 0;
	ACK = 1;
};

//
// Unicast message sent by the unicast protocol
//
packet UnicastMessage {
    //destination MAC address
    int destination;
    //source MAC address
    int source;
    //sequence number
    int sequenceNumber;
    //message type, data or ack
    int type enum(UnicastMessageType);
    //priority for the message (will be then mapped onto an AC)
    //0 = AC_BK
    //1 = AC_BE
    //2 = AC_VI
    //3 = AC_VO
    int priority = 0;
    //receive power in dBm
    double recvPower_dBm;
    //channel number
    int channel;
}
