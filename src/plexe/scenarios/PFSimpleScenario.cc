#include "plexe/scenarios/PFSimpleScenario.h"
#include "plexe/apps/PFApp.h"

namespace plexe {

Define_Module(PFSimpleScenario);

void PFSimpleScenario::initialize(int stage)
{
    ManeuverScenario::initialize(stage);
    if (stage == 1) {
        p1speed = par("p1speed").doubleValueInUnit("mps");
        p2speed = par("p2speed").doubleValueInUnit("mps");
        scenarioName = par("scenarioName").stringValue();
        startAdvertising = new cMessage("startAdvertising");
    }
    if (stage == 3) {
        pfapp = check_and_cast<PFApp*>(this->getModuleByPath("^.appl"));

        if (positionHelper->isLeader()) {
            plexeTraciVehicle->setActiveController(ACC);
            plexeTraciVehicle->setACCHeadwayTime(par("insertionHeadwayTime").doubleValue());

            for (int i = 1; i < positionHelper->getPlatoonSize(); i++) {
                std::stringstream ss;
                ss << par("platooningVType").stringValue() << "." << positionHelper->getMemberId(i);
                plexeTraciVehicle->addPlatoonMember(ss.str(), i);
            }

            // KEEP ALL VEH BLOCKED into INSERTION LANE  waiting for "startPFAppTime"
            plexeTraciVehicle->enableAutoLaneChanging(false);

            // adjust speed according to platoonId and scenarioName
            if (positionHelper->getPlatoonId() == 0) {
                plexeTraciVehicle->setCruiseControlDesiredSpeed(p1speed);
                positionHelper->setPlatoonSpeed(p1speed);
            }
            else if (positionHelper->getPlatoonId() == 1) {
                plexeTraciVehicle->setCruiseControlDesiredSpeed(p2speed);
                positionHelper->setPlatoonSpeed(p2speed);
            }

            //schedule startPFApp
            double waitPF = par("startAdvertising").doubleValueInUnit("s") + uniform(0.0, 0.1);
            scheduleAt(simTime() + waitPF, startAdvertising);

            // but initially start just in SEEK mode
            pfapp->setPFstate(PFState::SEEK);
            pfapp->startListening();
        }
        else {
            pfapp->setPFstate(PFState::STD_CAM);
            plexeTraciVehicle->setCruiseControlDesiredSpeed(positionHelper->getPlatoonSpeed() + 30);
        }
    }
}

void PFSimpleScenario::handleMessage(cMessage* msg)
{
    if (msg == startAdvertising) {
        if (scenarioName == "simple2") {
            // only P2 should advertise in scenario simple2
            if (positionHelper->getPlatoonId() == 0)
                return;
        }
        EV << "PFSIMPLESCENARIO " << positionHelper->getId()
            << " leading Platoon=" << positionHelper->getPlatoonId()
            << " startAdvertising()\n";
        pfapp->startAdvertising();
    }
}

PFSimpleScenario::~PFSimpleScenario()
{
    cancelAndDelete(startAdvertising);
    startAdvertising = nullptr;
}


} // namespace plexe
