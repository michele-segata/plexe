#pragma once

#include "plexe/mobility/TraCIBaseTrafficManager.h"
#include <string>
#include <vector>

namespace plexe {

class SimpleTrafficManager : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage) override;

    SimpleTrafficManager()
    {
        insertTrafficMessage = nullptr;
        insertSpeed = 0;
    }
    virtual ~SimpleTrafficManager();

protected:
    cMessage* insertTrafficMessage;
    double insertSpeed;
    std::string platooningVType;
    std::string lane0;
    std::string lane1;
    std::string lane2;
    double p1startPos;
    double p2startPos;

    double platoonId = 0;
    int vehicleId = 0;

    virtual void handleSelfMsg(cMessage* msg) override;
    void insertCar(std::string route, int lane, double speed,
        double desiredSpeed, double position, enum ACTIVE_CONTROLLER activeController, int platoonId, int platoPosition);
    void insertPlatoon(std::string platoString, int lane);

};

} // namespace plexe