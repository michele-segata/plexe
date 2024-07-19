//
// Created by stefano on 16/10/23.
//

#ifndef PLEXE_PLATOFORMTRAFFICINJECTIONPROCESS_H
#define PLEXE_PLATOFORMTRAFFICINJECTIONPROCESS_H

#include <omnetpp.h>
#include "plexe/traffic/PlatoformTrafficManager.h"
using namespace omnetpp;

namespace plexe {

class PlatoformTrafficInjectionProcess : public cSimpleModule {
protected:
    void initialize() override;
    void scenarioLoaded();
    void handleMessage(cMessage* msg) override;
    ~PlatoformTrafficInjectionProcess();

private:
    PlatoformTrafficManager* traffic;
    int lane = 0;
    int totLanes = 0;

    // max 2 elements, first one is the dontleave route and the second is the leave route (if present)
    double delay = 0.0;
    double alpha = 0.0;
    double headwayTime = 1.2;
    double vr = 0.0; // vehicleRate in vehs/minute
    double penetrationRate = 0.0;
    double minSpeed = 0.0, maxSpeed = 0.0;
    double avgSpeed = 0.0;
    double stepSpeed = 0.0;
    double minInsertionSpace = 6.0;
    double insertionSpeed = 25;
    double minGenInterval;

    std::unique_ptr<cMessage> insertCarMessage;

    friend class PlatoformTrafficManager;
};

Define_Module(PlatoformTrafficInjectionProcess);

} // namespace plexe

#endif // PLEXE_PLATOFORMTRAFFICINJECTIONPROCESS_H
