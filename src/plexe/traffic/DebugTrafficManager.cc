#include "DebugTrafficManager.h"
#include <string>

namespace plexe {

Define_Module(DebugTrafficManager);

void DebugTrafficManager::initialize(int stage)
{

    TraCIBaseTrafficManager::initialize(stage);

    if (stage == 0) {
        insertSpeed = par("insertionSpeed").doubleValueInUnit("mps");
        platooningVType = par("platooningVType").stdstringValue();

        insertTrafficMessage = new cMessage("insert cars");
        scheduleAt(1.0, insertTrafficMessage);
    }
}

void DebugTrafficManager::insertCar(std::string route, int lane, double speed,
    double desiredSpeed, bool isCommEnabled, double position)
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
        .headway = 1.2,
        .id = this->vehicleId,
        .platoonId = this->vehicleId++,
        .position = 0,
    };
    PlatoonInfo platoon_info{
        .speed = traci_info.speed,
        .lane = traci_info.lane};

    this->positions.addVehicleToPlatoon(vehicle_info.id, vehicle_info);
    this->positions.setPlatoonInformation(vehicle_info.platoonId, platoon_info);
}

void DebugTrafficManager::parseCarPositions(std::string parstringpos)
{
    strCarPositions = parstringpos;
    // std::cout << "strCarPositions = " << strCarPositions << "\n";
    std::vector<std::string> vecs = cStringTokenizer(strCarPositions.c_str()).asVector();
    carPositions.clear();
    for (auto sp : vecs)
        this->carPositions.push_back(std::stod(sp));
}

void DebugTrafficManager::handleSelfMsg(cMessage* msg)
{
    std::string s;
    double minSpeed = 90 / 3.6;
    double maxSpeed = 130 / 3.6;
    int lane = 0;
    if (msg == insertTrafficMessage) {
        s = par("carPositions0").stringValue();
        if (!s.empty()) {
            lane = 0;
            parseCarPositions(s);
            for (auto carpos : carPositions)
                insertCar(std::string("E0"), lane, insertSpeed, uniform(minSpeed, maxSpeed), true, carpos);
        }
        s = par("carPositions1").stringValue();
        if (!s.empty()) {
            lane = 1;
            parseCarPositions(s);
            for (auto carpos : carPositions)
                insertCar(std::string("E0"), lane, insertSpeed, uniform(minSpeed, maxSpeed), true, carpos);
        }
        s = par("carPositions2").stringValue();
        if (!s.empty()) {
            lane = 2;
            parseCarPositions(s);
            for (auto carpos : carPositions)
                insertCar(std::string("E0"), lane, insertSpeed, uniform(minSpeed, maxSpeed), true, carpos);
        }
    }
}

DebugTrafficManager::~DebugTrafficManager()
{
    cancelAndDelete(insertTrafficMessage);
    insertTrafficMessage = nullptr;
}

} // namespace plexe