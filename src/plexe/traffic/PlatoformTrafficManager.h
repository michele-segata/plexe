//
// Created by stefano on 9/13/23.
//

#ifndef PLATOFORM_TRAFFIC_MANAGER_H
#define PLATOFORM_TRAFFIC_MANAGER_H

#include <plexe/mobility/TraCIBaseTrafficManager.h>

namespace plexe {
class PlatoformTrafficManager : public TraCIBaseTrafficManager {
public:
    void initialize(int stage) override;
    ~PlatoformTrafficManager() = default;
    PlatoformTrafficManager() = default;
    void insertCar(std::string route, int lane, double speed, double desiredSpeed, bool isCommEnabled, double position);

protected:
    void handleSelfMsg(cMessage* msg) override;
    void scenarioLoaded() override;

private:
    int vehicleId = 0;
    bool trafficPreloading;
};

} // namespace plexe

#endif // PLATOFORM_TRAFFIC_MANAGER_H
