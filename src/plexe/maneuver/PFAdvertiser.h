#ifndef PLEXE_PFADVERTISER_H
#define PLEXE_PFADVERTISER_H

#include "plexe/maneuver/PFModule.h"

namespace plexe {

class PFApp;

enum class AdvState {
    SLEEPING,
    JOINWAITING,
    MERGING_LEADER,
};

class PFAdvertiser : public PFModule {

public:
    virtual ~PFAdvertiser();
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage* msg) override;
    using PFModule::enable;
    virtual void enable(int requesterId, int sessionId);
    virtual void disable() override;
    virtual void abortAdvertiser();

    virtual void onPFMessage(PFMessage* pfmsg) override;
    virtual void reset() override;
    virtual PFResponse* createPFResponse(PFResponseCode code, int sessionId);
    virtual bool checkPFRequestFeasible(const PFRequest* msg);

protected:
    AdvState advstate;
    int requesterId = -1;

    cMessage* timeout;
    double timeoutDuration;

    virtual void handlePFKeepAlive(const PFKeepAlive* msg);
    virtual void handlePFReadyToJoinNotification(const PFReadyToJoinNotification* msg);
    virtual void handlePFComplete(const PFComplete* msg);
    virtual void handlePFAbort(const PFAbort* msg);

    virtual PFJoinAuthorization* createPFJoinAuthorization(int status, int maneuverType, int sessionId);
    virtual PFUpdate* createPFUpdate(int sessionId);
    virtual PFCompleteAck* createPFCompleteAck(int sessionId);
};

} // namespace plexe

#endif // PLEXE_PFADVERTISER_H

