#include "PlatoformProtocol.h"

namespace plexe {

Define_Module(PlatoformProtocol);

void PlatoformProtocol::initialize(int stage)
{
    BaseProtocol::initialize(stage);

    if (stage == 0) {
        // random start time
        if (beaconingInterval > 0) {
            SimTime beginTime = SimTime(uniform(0.001, beaconingInterval));
            scheduleAt(simTime() + beaconingInterval + beginTime, sendBeacon);
        }
    }
    if (stage == 3) {
        pfapp = check_and_cast<PFApp*>(this->getModuleByPath("^.appl"));
    }
}

void PlatoformProtocol::handleSelfMsg(cMessage* msg)
{
    if (msg == sendBeacon) {
        if (this->platoonAvailable)
            this->sendPlatoAdvertisingBeacon(ProtocolCAMScope::BROADCAST, PlexeRadioInterfaces::VEINS_11P);
        else
            BaseProtocol::sendPlatooningMessage(ProtocolCAMScope::BROADCAST, PlexeRadioInterfaces::VEINS_11P);
        scheduleAt(simTime() + beaconingInterval, sendBeacon);
    }
    else {
        BaseProtocol::handleSelfMsg(msg);
    }
}

PFAdvertisement* PlatoformProtocol::createPlatoAdvertismentBeacon()
{
    // create platooning beacon including PlatoForm AdvertisingInfo
    PFAdvertisement* pkt = new PFAdvertisement();
    pkt->setPlatoonId(positionHelper->getPlatoonId());
    PFState parentPfstate = pfapp->getPFstate();
    bool canBeLeader = (parentPfstate == PFState::ADV_SEEK);
    pkt->setCanBeLeader(canBeLeader);

    pkt->setCcDesiredSpeed(pfapp->getPlexeTraciVehicle()->getCruiseControlDesiredSpeed());
    pkt->setMinToleratedSpeed(pfapp->getMinToleratedSpeed());
    pkt->setMaxToleratedSpeed(pfapp->getMaxToleratedSpeed());

    pkt->setCurrentPlatoonSize(positionHelper->getPlatoonSize());
    pkt->setCurrentLane(pfapp->getTraciVehicle()->getLaneIndex());
    pkt->setManeuverCapabilitiesCode(getModuleByPath("^.pfadv")->par("maneuverCapabilities").intValue());
    std::string cp = "Next Check Point (not defined yet)";
    pkt->setNextCheckpoint(cp.c_str());
    pkt->setNegotiationChannel(4); // should be chosen by each advertiser
    BaseProtocol::pupulateBeacon(pkt);
    return pkt;
}

void PlatoformProtocol::sendPlatoAdvertisingBeacon(int destinationAddress, enum PlexeRadioInterfaces interfaces)
{
    sendTo(encapsulateBeacon(createPlatoAdvertismentBeacon(), destinationAddress).release(), interfaces);
}

} // namespace plexe
