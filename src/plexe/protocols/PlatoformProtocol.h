#ifndef PLEXE_PLATOFORMPROTOCOL_H
#define PLEXE_PLATOFORMPROTOCOL_H

#include <plexe/protocols/BaseProtocol.h>
#include "plexe/messages/PFAdvertisement_m.h"
#include "plexe/apps/PFApp.h"

namespace plexe {

class PFApp;

class PlatoformProtocol : public BaseProtocol {

protected:
    virtual void handleSelfMsg(cMessage* msg) override;
    PFApp* pfapp;

private:
    bool platoonAvailable = false;
    bool ecamSubscribed = false;

public:
    PlatoformProtocol() {this->platoonAvailable = false;}
    virtual void initialize(int stage) override;

    virtual bool isPlatoonAvailable() { return this->platoonAvailable; }
    virtual void setPlatoonAvailable(bool v) { this->platoonAvailable = v; }

    virtual bool isEcamSubscribed() { return ecamSubscribed; }
    virtual void subscribeToPFAdvertisment(bool v) { ecamSubscribed = v; }

    virtual PFAdvertisement* createPlatoAdvertismentBeacon();
    void sendPlatoAdvertisingBeacon(int destinationAddress, enum PlexeRadioInterfaces interfaces);
    virtual int numInitStages() const override
    {
        return 4;
    }
};

} // namespace plexe

#endif // PLEXE_PLATOFORMPROTOCOL_H
