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

#include "veins/modules/application/platooning/apps/JoinApp.h"

#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/messages/MacPkt_m.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "veins/modules/application/platooning/protocols/BaseProtocol.h"

using namespace Veins;

Define_Module(JoinApp);

void JoinApp::onPlatoonBeacon(const PlatooningBeacon* pb) {
	if (positionHelper->isInSamePlatoon(pb->getVehicleId())) {
		//if the message comes from the leader
		if (pb->getVehicleId() == positionHelper->getLeaderId())
			traciVehicle->setLeaderVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed());

		//if the message comes from the vehicle in front
		if (pb->getVehicleId() == positionHelper->getFrontId()) {
			//get front vehicle position
			Coord frontPosition(pb->getPositionX(), pb->getPositionY(), 0);
			//get my position
			Veins::TraCICoord traciPosition = mobility->getManager()->omnet2traci(mobility->getCurrentPosition());
			Coord position(traciPosition.x, traciPosition.y);
			//compute distance
			double distance = position.distance(frontPosition) - pb->getLength();
			traciVehicle->setFrontVehicleFakeData(pb->getControllerAcceleration(), pb->getAcceleration(), pb->getSpeed(), distance);
		}
	}
	BaseApp::onPlatoonBeacon(pb);
}

