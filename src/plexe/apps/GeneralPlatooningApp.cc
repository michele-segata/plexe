//
// Copyright (C) 2012-2021 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2021 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "plexe/apps/GeneralPlatooningApp.h"

#include "plexe/protocols/BaseProtocol.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"
#include "veins/base/utils/FindModule.h"

using namespace veins;

namespace plexe {

Define_Module(GeneralPlatooningApp);

void GeneralPlatooningApp::initialize(int stage)
{
    BaseApp::initialize(stage);

    if (stage == 1) {
        // connect maneuver application to protocol
        protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"), gate("lowerControlIn"), gate("lowerControlOut"));
        // register to the signal indicating failed unicast transmissions
        findHost()->subscribe(Mac1609_4::sigRetriesExceeded, this);

        std::string joinManeuverName = par("joinManeuver").stdstringValue();
        if (joinManeuverName == "JoinAtBack")
            joinManeuver = new JoinAtBack(this);
        else
            throw new cRuntimeError("Invalid join maneuver implementation chosen");

        std::string mergeManeuverName = par("mergeManeuver").stdstringValue();
        if (mergeManeuverName == "MergeAtBack")
            mergeManeuver = new MergeAtBack(this);
        else
            throw new cRuntimeError("Invalid merge maneuver implementation chosen");

        scenario = FindModule<BaseScenario*>::findSubModule(getParentModule());
    }
}

void GeneralPlatooningApp::handleSelfMsg(cMessage* msg)
{
    if (joinManeuver && joinManeuver->handleSelfMsg(msg)) return;
    if (mergeManeuver && mergeManeuver->handleSelfMsg(msg)) return;
    BaseApp::handleSelfMsg(msg);
}

bool GeneralPlatooningApp::isJoinAllowed() const
{
    return ((role == PlatoonRole::LEADER || role == PlatoonRole::NONE) && !inManeuver);
}

enum ACTIVE_CONTROLLER GeneralPlatooningApp::getController()
{
    return scenario->getController();
}

double GeneralPlatooningApp::getTargetDistance(double speed)
{
    return scenario->getTargetDistance(speed);
}

void GeneralPlatooningApp::startJoinManeuver(int platoonId, int leaderId, int position)
{
    ASSERT(getPlatoonRole() == PlatoonRole::NONE);
    ASSERT(!isInManeuver());

    JoinManeuverParameters params;
    params.platoonId = platoonId;
    params.leaderId = leaderId;
    params.position = position;
    joinManeuver->startManeuver(&params);
}

void GeneralPlatooningApp::startMergeManeuver(int platoonId, int leaderId, int position)
{
    ASSERT(getPlatoonRole() == PlatoonRole::LEADER);
    ASSERT(!isInManeuver());

    JoinManeuverParameters params;
    params.platoonId = platoonId;
    params.leaderId = leaderId;
    params.position = position;
    mergeManeuver->startManeuver(&params);
}

void GeneralPlatooningApp::sendUnicast(cPacket* msg, int destination)
{
    Enter_Method_Silent();
    take(msg);
    BaseFrame1609_4* frame = new BaseFrame1609_4("BaseFrame1609_4", msg->getKind());
    frame->setRecipientAddress(destination);
    frame->setChannelNumber(static_cast<int>(Channel::cch));
    frame->encapsulate(msg);
    // send unicast frames using 11p only
    PlexeInterfaceControlInfo* ctrl = new PlexeInterfaceControlInfo();
    ctrl->setInterfaces(PlexeRadioInterfaces::VEINS_11P);
    frame->setControlInfo(ctrl);
    sendDown(frame);
}

void GeneralPlatooningApp::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(msg);

    cPacket* enc = frame->getEncapsulatedPacket();
    ASSERT2(enc, "received a BaseFrame1609_4s with nothing inside");

    if (enc->getKind() == MANEUVER_TYPE) {
        ManeuverMessage* mm = check_and_cast<ManeuverMessage*>(frame->decapsulate());
        if (UpdatePlatoonData* msg = dynamic_cast<UpdatePlatoonData*>(mm)) {
            handleUpdatePlatoonData(msg);
            delete msg;
        }
        else if (UpdatePlatoonFormation* msg = dynamic_cast<UpdatePlatoonFormation*>(mm)) {
            handleUpdatePlatoonFormation(msg);
            delete msg;
        }
        else {
            onManeuverMessage(mm);
        }
        delete frame;
    }
    else {
        BaseApp::handleLowerMsg(msg);
    }
}

void GeneralPlatooningApp::handleUpdatePlatoonData(const UpdatePlatoonData* msg)
{
    if (getPlatoonRole() != PlatoonRole::FOLLOWER) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getVehicleId() != positionHelper->getLeaderId()) return;

    handleUpdatePlatoonFormation(msg);
    LOG << positionHelper->getId() << " changing platoon id from " << positionHelper->getPlatoonId() << " to " << msg->getNewPlatoonId() << "\n";
    positionHelper->setPlatoonId(msg->getNewPlatoonId());
}

void GeneralPlatooningApp::handleUpdatePlatoonFormation(const UpdatePlatoonFormation* msg)
{
    if (getPlatoonRole() != PlatoonRole::FOLLOWER) return;
    if (msg->getPlatoonId() != positionHelper->getPlatoonId()) return;
    if (msg->getVehicleId() != positionHelper->getLeaderId()) return;

    // update formation information
    LOG << positionHelper->getId() << " changing platoon formation: ";
    std::vector<int> f;
    for (unsigned int i = 0; i < msg->getPlatoonFormationArraySize(); i++) {
        f.push_back(msg->getPlatoonFormation(i));
        LOG << msg->getPlatoonFormation(i) << " ";
    }
    LOG << "\n";
    positionHelper->setPlatoonFormation(f);
}

void GeneralPlatooningApp::setPlatoonRole(PlatoonRole r)
{
    role = r;
}

void GeneralPlatooningApp::onPlatoonBeacon(const PlatooningBeacon* pb)
{
    joinManeuver->onPlatoonBeacon(pb);
    mergeManeuver->onPlatoonBeacon(pb);
    // maintain platoon
    BaseApp::onPlatoonBeacon(pb);
}

void GeneralPlatooningApp::onManeuverMessage(ManeuverMessage* mm)
{
    if (activeManeuver) {
        activeManeuver->onManeuverMessage(mm);
    }
    else {
        joinManeuver->onManeuverMessage(mm);
        mergeManeuver->onManeuverMessage(mm);
    }
    delete mm;
}

void GeneralPlatooningApp::fillManeuverMessage(ManeuverMessage* msg, int vehicleId, std::string externalId, int platoonId, int destinationId)
{
    msg->setKind(MANEUVER_TYPE);
    msg->setVehicleId(vehicleId);
    msg->setExternalId(externalId.c_str());
    msg->setPlatoonId(platoonId);
    msg->setDestinationId(destinationId);
}

UpdatePlatoonData* GeneralPlatooningApp::createUpdatePlatoonData(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& platoonFormation, int newPlatoonId)
{
    UpdatePlatoonData* msg = new UpdatePlatoonData("UpdatePlatoonData");
    fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setPlatoonSpeed(platoonSpeed);
    msg->setPlatoonLane(platoonLane);
    msg->setPlatoonFormationArraySize(platoonFormation.size());
    for (unsigned int i = 0; i < platoonFormation.size(); i++) {
        msg->setPlatoonFormation(i, platoonFormation[i]);
    }
    msg->setNewPlatoonId(newPlatoonId);
    return msg;
}

UpdatePlatoonFormation* GeneralPlatooningApp::createUpdatePlatoonFormation(int vehicleId, std::string externalId, int platoonId, int destinationId, double platoonSpeed, int platoonLane, const std::vector<int>& platoonFormation)
{
    UpdatePlatoonFormation* msg = new UpdatePlatoonFormation("UpdatePlatoonFormation");
    fillManeuverMessage(msg, vehicleId, externalId, platoonId, destinationId);
    msg->setPlatoonSpeed(platoonSpeed);
    msg->setPlatoonLane(platoonLane);
    msg->setPlatoonFormationArraySize(platoonFormation.size());
    for (unsigned int i = 0; i < platoonFormation.size(); i++) {
        msg->setPlatoonFormation(i, platoonFormation[i]);
    }
    return msg;
}

void GeneralPlatooningApp::receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details)
{
    if (id == Mac1609_4::sigRetriesExceeded) {
        BaseFrame1609_4* frame = check_and_cast<BaseFrame1609_4*>(value);
        ManeuverMessage* mm = check_and_cast<ManeuverMessage*>(frame->getEncapsulatedPacket());
        if (frame) {
            joinManeuver->onFailedTransmissionAttempt(mm);
            mergeManeuver->onFailedTransmissionAttempt(mm);
        }
    }
}

void GeneralPlatooningApp::scheduleSelfMsg(simtime_t t, cMessage* msg)
{
    scheduleAt(t, msg);
}

GeneralPlatooningApp::~GeneralPlatooningApp()
{
    delete joinManeuver;
    delete mergeManeuver;
}

} // namespace plexe
