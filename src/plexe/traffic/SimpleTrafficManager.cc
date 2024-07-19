#include "SimpleTrafficManager.h"
#include <string>

namespace plexe {

Define_Module(SimpleTrafficManager);

void SimpleTrafficManager::initialize(int stage)
{

    TraCIBaseTrafficManager::initialize(stage);

    if (stage == 0) {
        insertSpeed = par("insertionSpeed").doubleValueInUnit("mps");
        platooningVType = par("platooningVType").stdstringValue();

        insertTrafficMessage = new cMessage("insert cars");
        scheduleAt(1.0, insertTrafficMessage);
    }
}

void SimpleTrafficManager::insertCar(std::string route, int lane, double speed,
    double desiredSpeed, double position, enum ACTIVE_CONTROLLER activeController, int platoonId, int platoPosition)
{
    Vehicle traci_info = {
        .id = findVehicleTypeIndex(par("platooningVType").stdstringValue()),
        .lane = lane,
        .position = static_cast<float>(position),
    };
    traci_info.speed = (float) speed;
    traci_info.ccDesiredSpeed = desiredSpeed;
    traci_info.vehicleId = this->vehicleId;
    this->addVehicleToQueue(route, traci_info);

    VehicleInfo vehicle_info = {
        .controller = activeController, // initially vehs are all ACC
        .distance = platoPosition == 0 ? 2.0 : 5.0, // 5 valid only for CACC!
        .headway = activeController == CACC ? 0 : 1.2, // valid for everyone...
        .id = this->vehicleId++,
        .platoonId = platoonId,
        .position = platoPosition,
    };

    this->positions.addVehicleToPlatoon(vehicle_info.id, vehicle_info);

    if (platoPosition == 0) {
        PlatoonInfo platoon_info{.speed = speed, .lane = lane};
        this->positions.setPlatoonInformation(platoonId, platoon_info);
    }

}

void SimpleTrafficManager::handleSelfMsg(cMessage* msg)
{
    std::string platoString;
    if (msg == insertTrafficMessage) {
        platoString = par("lane0").stringValue();
        if (!platoString.empty())
            insertPlatoon(platoString, 0);

        platoString = par("lane1").stringValue();
        if (!platoString.empty())
            insertPlatoon(platoString, 1);

        platoString = par("lane2").stringValue();
        if (!platoString.empty())
            insertPlatoon(platoString, 2);
    }
}

void SimpleTrafficManager::insertPlatoon(std::string platoString, int lane)
{
    std::vector<int> vehIds = cStringTokenizer(platoString.c_str()).asIntVector();
    double currentPos = platoonId == 0 ? par("p1startPos").doubleValue(): par("p2startPos").doubleValue();

    for (int i = 0; i < vehIds.size(); i++) {
        enum ACTIVE_CONTROLLER ctrl = i == 0 ? ACC : strToController(par("controller").stringValue());
        insertCar(par("route").stringValue(), lane, insertSpeed,
            insertSpeed, currentPos, ctrl, platoonId, i);
        currentPos -= par("vlength").doubleValueInUnit("m") + 2 + 1.2 * insertSpeed;
    }
    platoonId++;
}

SimpleTrafficManager::~SimpleTrafficManager()
{
    cancelAndDelete(insertTrafficMessage);
    insertTrafficMessage = nullptr;
}

} // namespace plexe