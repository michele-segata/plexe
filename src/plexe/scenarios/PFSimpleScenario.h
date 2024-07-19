#ifndef PFSIMPLESCENARIO_H
#define PFSIMPLESCENARIO_H

#include "plexe/scenarios/ManeuverScenario.h"
#include "plexe/apps/PFApp.h"

namespace plexe {

class PFSimpleScenario : public ManeuverScenario {

public:
    virtual ~PFSimpleScenario();
    void initialize(int stage) override;
    void handleMessage(cMessage* msg) override;
    virtual int numInitStages() const override
    {
        return 5;
    }

    PFSimpleScenario() = default;

private:
    PFApp* pfapp;
    double p1speed;
    double p2speed;
    cMessage* startAdvertising;

    std::string scenarioName;
};

} // namespace plexe

#endif // PFSIMPLESCENARIO_H
