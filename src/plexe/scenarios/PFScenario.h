#ifndef PFSCENARIO_H
#define PFSCENARIO_H

#include "plexe/scenarios/ManeuverScenario.h"
#include "plexe/PlexeManager.h"
#include "plexe/apps/PFApp.h"

namespace plexe {

class PFScenario : public ManeuverScenario {
public:
    void initialize(int stage) override;
    virtual int numInitStages() const override
    {
        return 4;
    }
    using ManeuverScenario::receiveSignal;
    virtual void receiveSignal(cComponent* src, simsignal_t id, const char *value, cObject* details) override;

    PFScenario() = default;
protected:
    PFApp* pfapp;
    std::string humstart;
    std::string cooldown;
    std::string switchoff;

};

} // namespace plexe

#endif // PFSCENARIO_H
