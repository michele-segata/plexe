#include "PFScenario.h"

namespace plexe {

Define_Module(PFScenario);

void PFScenario::initialize(int stage)
{
    ManeuverScenario::initialize(stage);
    if (stage == 1) {
        findHost()->subscribe(TraCIScenarioManager::traciInductionLoopEntrySignal, this);
        findHost()->subscribe(TraCIScenarioManager::traciInductionLoopExitSignal, this);
        humstart = par("humstart").stringValue();
        cooldown = par("cooldown").stringValue();
        switchoff = par("switchoff").stringValue();
    }

    if (stage == 3) {
        pfapp = check_and_cast<PFApp*>(this->getModuleByPath("^.appl"));
        pfapp->setPFstate(PFState::STD_CAM);
        // KEEP ALL VEH BLOCKED into INSERTION LANE  waiting for "HighwayUnderMeasure (hum)"
        plexeTraciVehicle->enableAutoLaneChanging(false);
        if (positionHelper->isLeader()) {
            this->plexeTraciVehicle->setActiveController(ACC);
            this->plexeTraciVehicle->setACCHeadwayTime(par("insertionHeadwayTime").doubleValue());
            plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed());

            for (int i = 1; i < positionHelper->getPlatoonSize(); i++) {
                std::stringstream ss;
                ss << par("platooningVType").stringValue() << "." << positionHelper->getMemberId(i);
                plexeTraciVehicle->addPlatoonMember(ss.str(), i);
            }
        }
        else {
            plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed() + 30);
        }
    }
}

void PFScenario::receiveSignal(cComponent* src, simsignal_t id, const char *value, cObject* details)
{
    Enter_Method_Silent();
    if (id == TraCIScenarioManager::traciInductionLoopEntrySignal) {
        std::string barrierName = value;

        if (barrierName.rfind(humstart, 0) == 0) {
            //We entered the Highway Under Measure sector, we should start advertising
            if (positionHelper->isLeader()) {
                plexeTraciVehicle->enableAutoLaneChanging(true);
                EV << positionHelper->getId() << ": starts Advertising at time: " << simTime() << std::endl;
                if (pfapp->getPFstate() == PFState::STD_CAM)
                    pfapp->startAdvertising();
            }
        } else if (barrierName.rfind(cooldown, 0) == 0) {
            if (pfapp->isCooldownPassed())
                return;
            pfapp->setCooldownPassed(true);
            pfapp->dumpStats(COOLDOWNBARRIER);
            if (positionHelper->isLeader()) {
                if (!pfapp->isActive()) {
                    if (pfapp->getPFstate() == PFState::ADV_SEEK)
                        pfapp->stopAdvertising();
                    if (pfapp->getPFstate() == PFState::SEEK)
                        pfapp->stopListening();
                    pfapp->setPFstate(PFState::STD_CAM);
                } else {
        //active veh at cooldown barrier. (Requester) abort if you can! We can't only iif
        // 1) we have mergeManeuver->checkDistance currently scheduled, which means we are closing the gap
                    if (pfapp->pfreq->isActive()) {
                        MergeAtBack* mm = dynamic_cast<MergeAtBack*>(pfapp->getMergeManeuver());
                        if (!mm->checkDistance->isScheduled()) {
                            pfapp->pfreq->abortRequester(
                                    PFTermCode::BARRIERCOOLDOWN, pfapp->pfreq->targetLeader);
                        } else {
                            pfapp->pfreq->dumpOperation(plexe::PFTermCode::BARRIERCOOLDOWN);
                        }
                    }
                }
            }
        } else if (barrierName.rfind(switchoff, 0) == 0) {
            if (pfapp->isSwitchoffPassed())
                return;
            pfapp->dumpStats(SWITCHOFFBARRIER);
            pfapp->pfreq->dumpOperation(plexe::PFTermCode::BARRIERSWITCHOFF);
            pfapp->setSwitchoffPassed(true);
            pfapp->setAllowDumpingStats(false);
            if (positionHelper->isLeader()) {
                if (!pfapp->isActive()) {
                    if (pfapp->getPFstate() == PFState::ADV_SEEK)
                        pfapp->stopAdvertising();
                    if (pfapp->getPFstate() == PFState::SEEK)
                        pfapp->stopListening();
                    pfapp->setPFstate(PFState::STD_CAM);
                } else {
                    throw cRuntimeError("This vehicle is still active at the switchoff barrier!");
                }
            }
        }
        return;
    } else if (id == TraCIScenarioManager::traciInductionLoopExitSignal) {
        return;
    }
    ManeuverScenario::receiveSignal(src, id, value, details);
}


} // namespace plexe
