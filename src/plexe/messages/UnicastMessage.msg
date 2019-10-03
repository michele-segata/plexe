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
