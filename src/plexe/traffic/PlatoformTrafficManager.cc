#include "plexe/traffic/PlatoformTrafficManager.h"
#include "plexe/traffic/PlatoformTrafficInjectionProcess.h"

#include <utility>

namespace plexe {
void PlatoformTrafficManager::initialize(int stage)
{
    TraCIBaseTrafficManager::initialize(stage);
    if (stage == 1) {
        trafficPreloading = par("trafficPreloading").boolValue();
    }
}

void PlatoformTrafficManager::handleSelfMsg(cMessage* msg)
{
    TraCIBaseTrafficManager::handleSelfMsg(msg);
}

void PlatoformTrafficManager::scenarioLoaded()
{
    TraCIBaseTrafficManager::scenarioLoaded();

    for (cModule::SubmoduleIterator it(this); !it.end(); ++it) {
        if (auto submodule = dynamic_cast<PlatoformTrafficInjectionProcess*>(*it)) {
            submodule->scenarioLoaded();
        }
    }
}

void PlatoformTrafficManager::insertCar(std::string route, int lane, double speed, double desiredSpeed, bool isCommEnabled, double position)
{
    Vehicle traci_info = {
        .id = findVehicleTypeIndex(isCommEnabled ? par("platooningVType").stdstringValue() :
                par("noCommVType").stdstringValue()),
        .lane = lane,
        .position = static_cast<float>(position),
    };
    traci_info.speed = (float) speed;
    traci_info.ccDesiredSpeed = desiredSpeed;
    this->addVehicleToQueue(route, traci_info);
    // if without communication stop here
    if (!isCommEnabled) return;
    // otherwise pupoulate also Plexe managers
    VehicleInfo vehicle_info = {
        .controller = ACC, // initially vehs are all ACC
        .distance = 5, // if ever used by a PATH veh...
        .headway = par("insertionHeadwayTime").doubleValue(),
        .id = this->vehicleId,
        .platoonId = this->vehicleId++,
        .position = 0,
    };
    PlatoonInfo platoon_info{
        .speed = traci_info.ccDesiredSpeed,
        .lane = traci_info.lane};

    this->positions.addVehicleToPlatoon(vehicle_info.id, vehicle_info);
    this->positions.setPlatoonInformation(vehicle_info.platoonId, platoon_info);
}

Define_Module(PlatoformTrafficManager);

} // namespace plexe
