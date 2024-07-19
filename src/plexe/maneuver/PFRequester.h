#ifndef PLEXE_PFREQUESTER_H
#define PLEXE_PFREQUESTER_H

#include "plexe/maneuver/PFModule.h"

namespace plexe {

class PFApp;
enum class PFOp;

enum class ReqState {
    SLEEPING,
    REQUESTING,
    MOVINGTOLANE,
    READYTOJOIN,
    MERGING_FOLLOWER,
    WAITCOMPLETEACK,
    //    FOLLOWER, //aka SIMPLEBEACONING
};

class PFRequester : public PFModule {

public:
    virtual ~PFRequester();
    virtual void initialize(int stage) override;
    using PFModule::enable;
    virtual void enable(int requestedPlatoonId, int requestedLeaderId);
    virtual void handleMessage(cMessage* msg) override;
    virtual void abortRequester(plexe::PFTermCode termcode, int abortMsgDestination);

    virtual void onPFMessage(PFMessage* pfmsg) override;
    virtual void handlePFCompleteAck(const PFCompleteAck* msg);

    virtual void reset() override;
    virtual void disable() override;
    virtual void finish() override;
    virtual bool checkAdvertisementFeasible(const PFAdvertisement* msg);
    virtual void dumpOperation(plexe::PFTermCode termcode);

    int targetLeader = -1;
protected:
    ReqState reqstate;

    int targetLane = -1;
    int targetVeh = -1;


    cMessage* timeout;
    double timeoutDuration;


    cMessage* pfcheck;
    cMessage* prepareTimeout;

    virtual void proactiveLaneChange(int targetLane);
    virtual unsigned int newSessionId(unsigned int maxvalue = 4294967295); // 2**32 - 1


    virtual void handlePFResponse(const PFResponse* msg);
    virtual void handlePFJoinAuthorization(const PFJoinAuthorization* msg);
    virtual void handlePFAbort(const PFAbort* msg);

    virtual PFRequest* createPFRequest(int targetPlatoonId, int sessionId);
    virtual PFKeepAlive* createPFKeepAlive(int sessionId);
    virtual PFReadyToJoinNotification* createPFReadyToJoinNotification(int vehInFrontId, int sessionId);
    virtual PFComplete* createPFComplete(int sessionId);

private:
    virtual void resetTimers();
};


} // namespace plexe

#endif // PLEXE_PFREQUESTER_H

