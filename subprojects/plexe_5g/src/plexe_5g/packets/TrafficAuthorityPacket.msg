//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//


//
// IceAlert Packet used by UEIceAlertApp MEIceAlertApp and MEIceAlertService
//

import nodes.mec.MECPlatform.MECPackets;

namespace plexe;

class TrafficAuthorityAppPacket extends simu5g::MECPacket
{
}

class TrafficAuthorityPacket extends TrafficAuthorityAppPacket
{
}

class TrafficAuthorityStartPacket extends TrafficAuthorityAppPacket
{
}

class TrafficAuthorityStopPacket extends TrafficAuthorityAppPacket
{
}
