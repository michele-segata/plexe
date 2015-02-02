//
// Copright (c) 2012-2015 Michele Segata <segata@ccs-labs.org>
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

#ifndef BASEAPP_H_
#define BASEAPP_H_

#include "veins/base/modules/BaseApplLayer.h"

#include "veins/modules/application/platooning/UnicastProtocol.h"
#include "veins/modules/application/platooning/messages/PlatooningBeacon_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/modules/application/platooning/CC_Const.h"

class BaseApp : public BaseApplLayer
{

	public:

		virtual void initialize(int stage);
		virtual void finish();

		/**
		 * Returns the traci external id of this car
		 */
		std::string getExternalId();

	protected:
		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void onData(WaveShortMessage* wsm);

	protected:

		Veins::TraCIMobility* mobility;
		Veins::TraCICommandInterface *traci;
		Veins::TraCICommandInterface::Vehicle *traciVehicle;

		//id of this vehicle
		int myId;

		//determine whether to send the actual acceleration or the one just computed by the controller
		bool useControllerAcceleration;
		//controller and engine related parameters
		double caccXi, caccOmegaN, caccC1, engineTau;
		//controller parameters for Ploeg's CACC
		double ploegH, ploegKp, ploegKd;
		double myccKd, myccKs;

		//speed and acceleration requested from traci at the last polling cycle
		double currentSpeed, currentAcceleration, currentControllerAcceleration;

	public:
		BaseApp()
		{}

		/**
		 * Sends a unicast message
		 *
		 * @param msg message to be encapsulated into the unicast message
		 * @param destination id of the destination
		 */
		void sendUnicast(cPacket *msg, int destination);

//    virtual ~BeaconingApp();

	protected:

		virtual void handleLowerMsg(cMessage *msg);
		virtual void handleSelfMsg(cMessage *msg);
		virtual void handleLowerControl(cMessage *msg);



};

#endif /* BASEAPP_H_ */
