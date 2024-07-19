import PFMessage;

packet PFResponse extends PFMessage {
    int status;
    int joinLane;
    int targetPlatoonMemberId;
    int chosenManeuverType;
    //int sessionId; inherited from PFMessage
}