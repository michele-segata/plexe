#ifndef PLEXE_PFMODULE_H
#define PLEXE_PFMODULE_H


#include <omnetpp.h>
#include "plexe/messages/PFMessage_m.h"
#include "plexe/utilities/BasePositionHelper.h"

#include "plexe/messages/PFAbort_m.h"
#include "plexe/messages/PFAdvertisement_m.h"
#include "plexe/messages/PFComplete_m.h"
#include "plexe/messages/PFCompleteAck_m.h"
#include "plexe/messages/PFInternalNotification_m.h"
#include "plexe/messages/PFJoinAuthorization_m.h"
#include "plexe/messages/PFKeepAlive_m.h"
#include "plexe/messages/PFReadyToJoinNotification_m.h"
#include "plexe/messages/PFRequest_m.h"
#include "plexe/messages/PFResponse_m.h"
#include "plexe/messages/PFUpdate_m.h"
#include "plexe/messages/PFInternalNotification_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "plexe/mobility/CommandInterface.h"
#include "plexe/utilities/BasePositionHelper.h"

#define EPS (0.01/3.6)

using namespace omnetpp;

namespace plexe {

class PFApp;

enum class PFTermCode {
    REQUEST_SENT,
    ABORT_COORD_TIMEOUT,
    COMPLETE_COORD,
    ABORT_INTRUDERDETECTED,
    ABORT_TOOLONGLANECHANGE,
    ABORT_REQUESTER_TIMEOUT,
    RESP_DENIED,
    AUTH_DENIED,
    PF_COMPLETED,
    ABORTMSGRECEIVED,
    TX_FAILURE,
    PFREQUEST_TX_FAILURE,
    END_SIMULATION,
    BARRIERCOOLDOWN,
    BARRIERSWITCHOFF,
    ABORT_CLOSE_TO_END_ROUTE,
};

class PFModule : public omnetpp::cSimpleModule {
public:
    virtual void initialize(int stage) override;
    virtual ~PFModule();
    int numInitStages() const override { return 3; }

    virtual void onPFMessage(PFMessage* pfmsg) { throw cRuntimeError("unimplemented onPFMessage\n"); };     // (message delegation from driver->PFApp->PFModule)
    virtual void reset() { throw cRuntimeError("unimplemented reset\n"); };     // clear internal PFModule (on comeplete or abort)

    virtual bool isActive() { Enter_Method_Silent(); return active; };     // report active flag status
    virtual void enable(); // raise active flag
    virtual void disable() {active = false;};     // lower active flag

    virtual void freezeLane();     // (PFReq and PFAdv must be ablto to freeze/un- lane)
    virtual void unfreezeLane();
    std::string termCode2string(PFTermCode);

    virtual unsigned int getSessionId() { return this->sessionId; }; // while isActive() a current session ID must have been nogotiated

protected:
    PFApp* pfapp;
    BasePositionHelper* positionHelper;
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    traci::CommandInterface* plexeTraci;
    std::unique_ptr<traci::CommandInterface::Vehicle> plexeTraciVehicle;

    bool active = false;

    unsigned int sessionId = INVALID_SESSION;
    simtime_t activationTime = -1;
    double activationPosX = -1;
    void sendPFNotification(PFTermCode notice);
    PFInternalNotification* createPFInternalNotification(PFTermCode notice);
    virtual PFAbort* createPFAbort(int sessionId, std::string reason = "ABORT!");
};
} // namespace plexe

#endif // PLEXE_PFMODULE_H
