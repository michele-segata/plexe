#pragma once

#include "plexe/mobility/TraCIBaseTrafficManager.h"
#include <string>
#include <vector>

namespace plexe {

class DebugTrafficManager : public TraCIBaseTrafficManager {

public:
    virtual void initialize(int stage) override;

    DebugTrafficManager()
    {
        insertTrafficMessage = nullptr;
        insertSpeed = 0;
        trafficInsertTime = SimTime(0);
    }
    virtual ~DebugTrafficManager();

protected:
    // this is used to start traffic generation
    cMessage* insertTrafficMessage;
    SimTime trafficInsertTime;

    double insertSpeed;
    std::string platooningVType;
    std::string strCarPositions;
    std::vector<double> carPositions;

    virtual void handleSelfMsg(cMessage* msg) override;
    void parseCarPositions(std::string parstringpos);
    void insertCar(std::string route, int lane, double speed, double desiredSpeed, bool isCommEnabled, double position);

private:
    int vehicleId = 0;
};

} // namespace plexe