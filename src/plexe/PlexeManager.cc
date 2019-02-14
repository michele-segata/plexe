#include "PlexeManager.h"

#include "plexe/mobility/CommandInterface.h"

namespace plexe {

Define_Module(PlexeManager);

void PlexeManager::initialize(int stage)
{
    const auto scenarioManager = Veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);

    if (scenarioManager->isTraciInitialized()) {
        initializeCommandInterface();
    }
    else {
        scenarioManager->subscribe(Veins::TraCIScenarioManager::traciInitializedSignal, &deferredCommandInterfaceInitializer);
    }
}

void PlexeManager::initializeCommandInterface()
{
    const auto scenarioManager = Veins::TraCIScenarioManagerAccess().get();
    ASSERT(scenarioManager);
    commandInterface.reset(new traci::CommandInterface(this, scenarioManager->getCommandInterface(), scenarioManager->getConnection()));
}

} // namespace plexe
