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

#ifndef JOINMANEUVERSCENARIO_H_
#define JOINMANEUVERSCENARIO_H_

#include "veins/modules/application/platooning/scenarios/BaseScenario.h"
#include "veins/modules/application/platooning/protocols/BaseProtocol.h"

#include "veins/modules/application/platooning/messages/ManeuverMessage_m.h"

class JoinManeuverScenario : public BaseScenario
{

	protected:
		//define the roles
		enum JOIN_ROLE {LEADER, FOLLOWER, JOINER};
		//data that each car needs to keep
		struct PLATOON_DATA {
			double				speed;		//speed of the platoon
			int					joinLane;	//the lane chosen for joining the platoon
			int					joinerId;	//the id of the vehicle joining the platoon
			std::vector<int>	formation;	//list of vehicles in the platoon
		};
		//define the states for each role
		typedef enum _LEADER_STATES {
			LS_LEADING,
			LS_WAIT_JOINER_IN_POSITION,
			LS_WAIT_JOINER_TO_JOIN
		} LEADER_STATES;
		typedef enum _JOINER_STATES {
			JS_IDLE,
			JS_WAIT_REPLY,
			JS_MOVE_IN_POSITION,
			JS_WAIT_JOIN,
			JS_JOIN_PLATOON,
			JS_FOLLOW
		} JOINER_STATES;
		typedef enum _FOLLOWER_STATES {
			FS_FOLLOW
		} FOLLOWER_STATES;
		//define the messages that can be sent by each role
		enum LEADER_MSGS {
			LM_MOVE_IN_POSITION = 0,
			LM_JOIN_PLATOON = 1,
			LM_UPDATE_FORMATION = 2
		};
		enum JOINER_MSGS {
			JM_REQUEST_JOIN = 3,
			JM_IN_POSITION = 4,
			JM_IN_PLATOON = 5
		};

		//the state machine handler
		int leaderFsm, joinerFsm, followerFsm;
		//the role of this vehicle
		JOIN_ROLE role;
		//the position of this vehicle in the platoon
		int position;
		//data known by the vehicle
		struct PLATOON_DATA platoonData;
		//message used to start the maneuver
		cMessage *startManeuver;
		//pointer to protocol
		BaseProtocol *protocol;

	public:

		static const int MANEUVER_TYPE = 12347;

		virtual void initialize(int stage);

	protected:
		void sendUnicast(cPacket *msg, int destination);

	private:

	public:
		JoinManeuverScenario()	{
			startManeuver = nullptr;
		}
		virtual ~JoinManeuverScenario();

	protected:

		virtual void handleSelfMsg(cMessage *msg);
		//override this method of BaseApp. we want to handle it ourself
		virtual void handleLowerMsg(cMessage *msg);
		virtual void handleLowerControl(cMessage *msg);

		ManeuverMessage *generateMessage();

		void handleLeaderMsg(cMessage *msg);
		void handleJoinerMsg(cMessage *msg);
		void handleFollowerMsg(cMessage *msg);

		void prepareManeuverCars(int platoonLane);

};

#endif
