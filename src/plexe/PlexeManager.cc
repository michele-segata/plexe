#include "PlexeManager.h"

#include "plexe/mobility/CommandInterface.h"

namespace plexe {

Define_Module(PlexeManager);

void PlexeManager::initialize(int stage)
{
    const auto scenarioManager = veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);

    scenarioManager->subscribe(veins::TraCIScenarioManager::traciTimestepEndSignal, &plexeTimestepTrigger);

    if (scenarioManager->isUsable()) {
        initializeCommandInterface();
    }
    else {
        scenarioManager->subscribe(veins::TraCIScenarioManager::traciInitializedSignal, &deferredCommandInterfaceInitializer);
    }
}

void PlexeManager::initializeCommandInterface()
{

    const auto scenarioManager = veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);
    commandInterface.reset(new traci::CommandInterface(this, scenarioManager->getCommandInterface(), scenarioManager->getConnection()));
}

} // namespace plexe
