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

#ifndef BASEPROTOCOL_H_
#define BASEPROTOCOL_H_

#include "veins/base/modules/BaseApplLayer.h"

#include "veins/modules/application/platooning/UnicastProtocol.h"
#include "veins/modules/application/platooning/messages/PlatooningBeacon_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

class BaseProtocol : public BaseApplLayer {

	private:

		//output vectors for statistics
		cOutVector distanceOut, relSpeedOut, nodeIdOut, speedOut, posxOut, posyOut, accelerationOut;

	protected:

		//id of this vehicle
		int myId;
		//sequence number of sent messages
		int seq_n;

		//beaconing interval (i.e., update frequency)
		SimTime beaconingInterval;
		//priority used for messages (i.e., the access category)
		int priority;
		//packet size of the platooning message
		int packetSize;
		//time at which simulation should stop after communication started
		SimTime communicationDuration;
		//determine whether to send the actual acceleration or the one just computed by the controller
		bool useControllerAcceleration;

		//input/output gates from/to upper layer
		int upperLayerIn, upperLayerOut, upperControlIn, upperControlOut, lowerLayerIn, lowerLayerOut;

		//messages for scheduleAt
		cMessage *sendBeacon, *dataPolling;

		/**
		 * NB: this method must be overridden by inheriting classes, BUT THEY MUST invoke the super class
		 * method prior processing the message. For example, the start communication event is handled by the
		 * BaseProtocol which then calls the startCommunications method. Also statistics are handled
		 * by BaseProtocol and are recorder periodically.
		 */
		virtual void handleSelfMsg(cMessage *msg);

		//TODO: implement method and pass info to upper layer (bogus platooning) as it is (msg)
		virtual void handleLowerMsg(cMessage *msg);

		//handle unicast messages coming from above layers
		virtual void handleUpperMsg(cMessage *msg);

		//handle control messages coming from above
		virtual void handleUpperControl(cMessage *msg);

		//handle control messages coming from below
		virtual void handleLowerControl(cMessage *msg);

		//handles and application layer message
		void handleUnicastMsg(UnicastMessage *unicast);

		/**
		 * Sends a platooning message with all information about the car. This is an utility function for
		 * subclasses
		 */
		void sendPlatooningMessage(int destinationAddress);

		/**
		 * This method must be overridden by subclasses to take decisions
		 * about what to do.
		 * Passed packet MUST NOT be freed, but just be read. Freeing is a duty of the
		 * superclass
		 *
		 * \param pkt the platooning beacon
		 * \param unicast the original unicast packet which was containing pkt
		 */
		virtual void messageReceived(PlatooningBeacon *pkt, UnicastMessage *unicast);

		//traci mobility. used for getting/setting info about the car
		Veins::TraCIMobility *mobility;
		Veins::TraCICommandInterface *traci;
		Veins::TraCICommandInterface::Vehicle *traciVehicle;

	public:

		//id for beacon message
		static const int BEACON_TYPE = 12345;

		BaseProtocol() {
			sendBeacon = 0;
			dataPolling = 0;
		}
		virtual ~BaseProtocol();

		virtual void initialize(int stage);
		virtual void finish();
};

#endif /* BASEPROTOCOL_H_ */
