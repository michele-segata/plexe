//
// Created by stefano on 16/10/23.
//
#include "plexe/traffic/PlatoformTrafficInjectionProcess.h"
#include <plexe/mobility/TraCIBaseTrafficManager.h>

#define ALPHA(rho, speed, minGenInterval) (((rho) * (speed)) / (1000 - (rho) * (speed) * (minGenInterval)))
#define SPEED(rho, headwayTime, minInsertionSpace, avgSpeed) (std::min(avgSpeed, (1000 - (minInsertionSpace * rho)) / (rho * headwayTime)))

namespace plexe {

void PlatoformTrafficInjectionProcess::initialize()
{
    this->traffic = static_cast<PlatoformTrafficManager*>(getModuleByPath("^"));
    // vr is configured in minutes, but we need it "per second"
    this->vr = this->par("vehicleRate").doubleValue();
    this->alpha = this->vr / 60; // alpha is now in veh/sec
    this->minSpeed = traffic->par("minSpeed").doubleValueInUnit("mps");
    this->maxSpeed = traffic->par("maxSpeed").doubleValueInUnit("mps");
    this->stepSpeed = traffic->par("stepSpeed").doubleValueInUnit("mps");
    this->insertionSpeed = traffic->par("insertionSpeed").doubleValueInUnit("mps");
    this->penetrationRate = this->traffic->par("penetrationRate").doubleValue();

    this->avgSpeed = (this->minSpeed + this->maxSpeed) / 2.0;
    this->headwayTime = this->traffic->par("insertionHeadwayTime").doubleValueInUnit("s");
    this->minInsertionSpace = this->traffic->par("minInsertionSpace").doubleValueInUnit("m");

    this->totLanes = this->traffic->par("entryLanes").intValue();

    this->minGenInterval = this->headwayTime + this->minInsertionSpace / this->insertionSpeed;

    this->delay = this->par("insertionDelay").doubleValue();
    this->lane = this->getIndex();

    this->insertCarMessage = std::make_unique<cMessage>((
                std::string("Insert car on lane ") + std::to_string(this->lane))
            .c_str());
    if (lane == 0) {
        std::cout << "insertSpeed = " << insertionSpeed << "\n"
            << "minGenInterval = " << minGenInterval << "\n"
            << "alpha [veh/sec] = " << alpha << "\n"
            << "vehRate [veh/min] = " << vr << "\n";
    }

}

void PlatoformTrafficInjectionProcess::scenarioLoaded()
{
    // only the first event of this lane
    auto firstEventTime = 0.0 + this->delay + uniform(0, 0.3);
    scheduleAt(firstEventTime, this->insertCarMessage.get());

    // Preload cars (filling the 5 km strecth from km15 to km0)
    if (traffic->par("trafficPreloading").boolValue()) {
        double firstStretchLength = 4875; // meters
        double distance = firstStretchLength - this->minInsertionSpace - uniform(0, 10);
        while (distance > 0) {
            this->traffic->insertCar(
                std::string("E0"),
                this->lane,
                this->insertionSpeed,
                uniform(this->minSpeed, this->maxSpeed),
                false, // all inizio vogliamo macchine senza radio
                // uniform(0, 1) < this->penetrationRate, //
                distance);

            // inter-car distance <- avgTimeInterval (= minGenInterlva + 1/alpha) x speed
            // [space = time*speed]
            distance -= (this->minGenInterval*this->insertionSpeed + uniform(0, 0.8)); // no need of zero-average sminking
        } // end of preloading loop
    }
}

void PlatoformTrafficInjectionProcess::handleMessage(cMessage* msg)
{
    if (msg == this->insertCarMessage.get()) {

        int x = intuniform(0, std::lround(((maxSpeed - minSpeed) / stepSpeed)));
        double ccDesiredSpeed = minSpeed + x * stepSpeed;
        if (ccDesiredSpeed < minSpeed)
            throw cRuntimeError("ccDesSpeed too low!");

        if (ccDesiredSpeed > maxSpeed)
            throw cRuntimeError("ccDesSpeed too high!");

        // 4 meter, as vehicle-length, to place vehicles completely inside the road
        double initialPos = 0.0;
        this->traffic->insertCar(
            std::string("E0"),
            this->lane,
            this->insertionSpeed,
            ccDesiredSpeed,
            uniform(0, 1) <= this->penetrationRate,
            initialPos);

        double avgInterArrivalTimePrime = (1/alpha - this->minGenInterval);
        auto deltaT = this->minGenInterval + exponential(avgInterArrivalTimePrime);

        scheduleAt(simTime() + deltaT, this->insertCarMessage.get());
        return;
    }
    delete msg;
}

PlatoformTrafficInjectionProcess::~PlatoformTrafficInjectionProcess()
{
    if (this->insertCarMessage)
        cancelEvent(this->insertCarMessage.get());
}

} // namespace plexe
