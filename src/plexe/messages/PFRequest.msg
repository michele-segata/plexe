import PFMessage;

packet PFRequest extends PFMessage {
    int targetPlatoonId;
    int requesterPlatoonSize;
    int requesterLane;
    double requesterCcDesSpeed;
    double minToleratedSpeed;
    double maxToleratedSpeed;
    int maneuverCapabilitiesCode;
    //int sessionId; inherited from PFMessage
}