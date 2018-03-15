//
// Copyright (c) 2012-2016 Michele Segata <segata@ccs-labs.org>
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

packet ManeuverMessage {
	//id of the originator
	int vehicleId;
	//platoon id
	int platoonId;
	//message type
	int messageType;
	//platoon speed
	double platoonSpeed;
	//platoon lane
	int platoonLane;
	//id of vehicle in front of the joiner (G)
	int frontVehicleId;
	//id of the follower vehicle (F)
	int followerVehicleId;
	//platoon formation
	int platoonFormation[];
}
