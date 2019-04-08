#pragma once

#include <plexe/plexe.h>

#include <veins/modules/mobility/traci/TraCIScenarioManager.h>
#include <veins/modules/mobility/traci/TraCICommandInterface.h>

#include <plexe/mobility/CommandInterface.h>

namespace plexe {

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
            : owner(owner)
        {
        }

        void receiveSignal(cComponent* source, simsignal_t signalID, bool b, cObject* details) override
        {
            ASSERT(signalID == veins::TraCIScenarioManager::traciInitializedSignal && b);

            owner->initializeCommandInterface();
        }

    private:
        PlexeManager* owner;
    };

    class PlexeTimestepTrigger : public cListener {
    public:
        PlexeTimestepTrigger(PlexeManager* owner)
            : owner(owner)
        {
        }

        void receiveSignal(cComponent*, simsignal_t signalID, const simtime_t&, cObject*) override
        {
            ASSERT(signalID == veins::TraCIScenarioManager::traciTimestepEndSignal);

            owner->getCommandInterface()->executePlexeTimestep();
        }

    private:
        PlexeManager* owner;
    };

    void initializeCommandInterface();

    DeferredCommandInterfaceInitializer deferredCommandInterfaceInitializer{this};
    PlexeTimestepTrigger plexeTimestepTrigger{this};
    std::unique_ptr<traci::CommandInterface> commandInterface;
};

} // namespace plexe
