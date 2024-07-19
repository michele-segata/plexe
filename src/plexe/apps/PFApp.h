
#ifndef PFAPP_H_
#define PFAPP_H_

#include "plexe/apps/GeneralPlatooningApp.h"
#include "plexe/driver/PlexeRadioDriverInterface.h"
#include "plexe/maneuver/PFModule.h"
#include "plexe/maneuver/PFRequester.h"
#include "plexe/maneuver/PFAdvertiser.h"
#include "plexe/protocols/PlatoformProtocol.h"
#include "plexe/traffic/PlatoformTrafficManager.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"

#include "plexe/maneuver/JoinAtBack.h"
#include <map>
#include <array>
#include <deque>
#include <set>
#include <fstream>

namespace plexe {

class PFHelper;
class PFModule;
class PlatoformProtocol;

enum PFState {
    STD_CAM,
    SEEK,
    ADV_SEEK,
    REQUESTER,
    COORD,
    GRACEP
};


struct PF_BACKUP_DATA {
    PFState pfstate = PFState::STD_CAM;
    PlatoonRole role = PlatoonRole::NONE;
};

class PFApp : public GeneralPlatooningApp {

public:
    cMessage* gracePeriod;

    virtual ~PFApp();
    virtual void initialize(int stage) override;
    int numInitStages() const override { return 6; }
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void finish() override;

    virtual void startAdvertising();
    virtual void stopAdvertising();
    virtual void startListening();
    virtual void stopListening();
    virtual bool isActive();
    static std::string pfs2string(PFState pfs);

    // https://stackoverflow.com/questions/18515183/c-overloaded-virtual-function-warning-by-clang
    using GeneralPlatooningApp::sendUnicast;
    virtual void sendUnicast(cPacket* msg, int destination, enum PlexeRadioInterfaces interfaces);
    virtual void sendUnicast(cPacket* msg, int destination) override;
    virtual void dumpStats(std::string optionalMessage = "");


    virtual double getMinToleratedSpeed() { return this->minToleratedSpeed; };
    virtual void setMinToleratedSpeed(double minToleratedSpeed) { this->minToleratedSpeed = minToleratedSpeed; };
    virtual double getMaxToleratedSpeed() { return this->maxToleratedSpeed; };
    virtual void setMaxToleratedSpeed(double maxToleratedSpeed) { this->maxToleratedSpeed = maxToleratedSpeed; };
    virtual JoinManeuver* getMergeManeuver() {
        return this->mergeManeuver;
    }
    void setAllowDumpingStats(bool allowDumpingStats = true) {
        this->allowDumpingStats = allowDumpingStats;
    }

    bool isAllowDumpingStats() const {
        return allowDumpingStats;
    }

    bool isCooldownPassed() const {
        return cooldown_passed;
    }

    void setCooldownPassed(bool cooldownPassed = false) {
        cooldown_passed = cooldownPassed;
    }

    bool isSwitchoffPassed() const {
        return switchoff_passed;
    }

    void setSwitchoffPassed(bool switchoffPassed = false) {
        switchoff_passed = switchoffPassed;
    }

    PFState getPFstate() const {
        return pfstate;
    }

    void setPFstate(PFState pfstate) {
        this->pfstate = pfstate;
        sPFstate = PFApp::pfs2string(pfstate);
    }

    PFRequester* pfreq;
    PFAdvertiser* pfadv;

    int maxPlatoSize;

    simsignal_t platoonId_sig;
    simsignal_t platoonSize_sig;
    simsignal_t platoVarX_sig;
    simsignal_t platoleaderFlag_sig;
    simsignal_t pfvehId_sig;
    
    simsignal_t pfvehSessId_sig;
    simsignal_t pfoperation_sig;
    simsignal_t pfsessionId_sig;
    simsignal_t pfcoordId_sig;
    simsignal_t pfsessionStartTime_sig;
    simsignal_t pfsessionEndTime_sig;
    simsignal_t pfsessionStartX_sig; 
    simsignal_t pfsessionEndX_sig;
    
    PF_BACKUP_DATA backupPoint;

protected:
    PlatoformProtocol* pfprotocol;
    PlatoformTrafficManager* traffic = nullptr;

    PFState pfstate;
    std::string sPFstate;
    PFState preGPstate;

    std::map<int, std::deque<double>> eCamHistory;
    std::set<std::string> crossedBarriers;

    unsigned int requiredValidAdvertisement;
    double maxAdvertiserDistance;
    double minAdvertiserDistance;
    double minToleratedSpeed = 90.0 / 3.6;
    double maxToleratedSpeed = 130.0 / 3.6;

    virtual void createBackupPoint();
    virtual void restoreBackupPoint();

    virtual PFModule* getActivePFModule();

    virtual void handleMessage(cMessage* msg) override;
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) override;
    virtual void handleLowerMsg(cMessage* msg) override;
    virtual void receiveSignal(cComponent* src, simsignal_t id, cObject* value, cObject* details) override;

private:
    void handlePFAdvertisement(const PFAdvertisement* msg);
    void handlePFRequest(const PFRequest* msg);
    virtual void handlePFUpdate(const PFUpdate* msg);
    virtual bool keepAdvertising();
    virtual bool keepListening();
    std::ofstream myfile;
    bool seqdiag = false;
    bool allowDumpingStats = true;
    bool cooldown_passed = false;
    bool switchoff_passed = false;
    double maxSpeed = 130.0 / 3.6;
};

} // namespace plexe

#endif /* PFAPP_H_ */
