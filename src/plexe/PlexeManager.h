#pragma once

#include <plexe/plexe.h>

#include <veins/modules/mobility/traci/TraCIScenarioManager.h>

namespace plexe {

namespace traci {
class CommandInterface;
}

class PlexeManager : public cSimpleModule {
public:
    void initialize(int stage) override;

    /**
     * Return a weak pointer to the CommandInterface owned by this manager.
     */
    traci::CommandInterface* getCommandInterface()
    {
        return commandInterface.get();
    }
private:
    class DeferredCommandInterfaceInitializer : public cListener {
    public:
        DeferredCommandInterfaceInitializer(PlexeManager* owner)
            : owner(owner) {}
        void receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) override
        {
            ASSERT(signalID == Veins::TraCIScenarioManager::traciInitializedSignal && b);

            owner->initializeCommandInterface();
        }
    private:
        PlexeManager* owner;
    };

    void initializeCommandInterface();

    DeferredCommandInterfaceInitializer deferredCommandInterfaceInitializer{this};
    std::unique_ptr<traci::CommandInterface> commandInterface;
};

} // namespace plexe
