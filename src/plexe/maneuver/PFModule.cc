#include "plexe/maneuver/PFModule.h"
#include "plexe/apps/PFApp.h"

using namespace omnetpp;

namespace plexe {

Define_Module(PFModule);

void PFModule::initialize(int stage)
{
    if (stage == 2) {
        pfapp = check_and_cast<PFApp*>(this->getModuleByPath("^.appl"));
        positionHelper = pfapp->getPositionHelper();
        mobility = pfapp->getMobility();
        traci = pfapp->getTraci();
        traciVehicle = pfapp->getTraciVehicle();
        plexeTraci = pfapp->getPlexeTraci();
        plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, mobility->getExternalId()));
    }
}

void PFModule::freezeLane()
{
    EV << "Freezing Lane\n";
    plexeTraciVehicle->enableAutoLaneChanging(false);
}

void PFModule::unfreezeLane()
{
    EV << "Unfreezing Lane\n";
    plexeTraciVehicle->enableAutoLaneChanging(true);
}

void PFModule::sendPFNotification(PFTermCode noticeCode)
{
    PFInternalNotification* pfnotice = createPFInternalNotification(noticeCode);
    send(pfnotice, "pfout");
}

PFInternalNotification* PFModule::createPFInternalNotification(PFTermCode noticeCode)
{
    PFInternalNotification* msg = new PFInternalNotification("PFInternalNotification");
    msg->setSessionId(sessionId);
    msg->setNotice((int) noticeCode);
    msg->setKind(PF_NOTIFICATION);
    return msg;
}

PFAbort* PFModule::createPFAbort(int sessionId, std::string reason)
{
    PFAbort* msg = new PFAbort("PFAbort");
    msg->setReason(reason.c_str());
    msg->setSessionId(sessionId);
    msg->setVehicleId(positionHelper->getId());
    msg->setKind(PF_TYPE);
    return msg;
}

std::string PFModule::termCode2string(PFTermCode tc)
{
    if (tc == PFTermCode::ABORT_COORD_TIMEOUT)
        return "ABORT_COORD_TIMEOUT";
    else if (tc == PFTermCode::COMPLETE_COORD)
        return "COMPLETE_COORD";
    else if (tc == PFTermCode::ABORT_INTRUDERDETECTED)
        return "ABORT_INTRUDERDETECTED";
    else if (tc == PFTermCode::ABORT_TOOLONGLANECHANGE)
        return "ABORT_TOOLONGLANECHANGE";
    else if (tc == PFTermCode::ABORT_REQUESTER_TIMEOUT)
        return "ABORT_REQUESTER_TIMEOUT";
    else if (tc == PFTermCode::RESP_DENIED)
        return "RESP_DENIED";
    else if (tc == PFTermCode::AUTH_DENIED)
        return "AUTH_DENIED";
    else if (tc == PFTermCode::PF_COMPLETED)
        return "PF_COMPLETED";
    else if (tc == PFTermCode::ABORTMSGRECEIVED)
        return "ABORTMSGRECEIVED";
    else if (tc == PFTermCode::TX_FAILURE)
        return "TX_FAILURE";
    else if (tc == PFTermCode::PFREQUEST_TX_FAILURE)
        return "PFREQUEST_TX_FAILURE";
    else if (tc == PFTermCode::END_SIMULATION)
        return "END_SIMULATION";
    else if (tc == PFTermCode::BARRIERCOOLDOWN)
        return "BARRIERCOOLDOWN";
    else if (tc == PFTermCode::BARRIERSWITCHOFF)
        return "BARRIERSWITCHOFF";
    else if (tc == PFTermCode::ABORT_CLOSE_TO_END_ROUTE)
        return "ABORT_CLOSE_TO_END_ROUTE";
    else
        throw cRuntimeError("Unknown PFTermCode");
}

void PFModule::enable()
{
    if (isActive())
        throw cRuntimeError("module is already active.");
    active = true;
    activationTime = simTime();
    auto coord = mobility->getPositionAt(simTime());
    activationPosX = coord.x;
}

PFModule::~PFModule()
{

}

} // namespace plexe
