import PFMessage;

packet PFReadyToJoinNotification extends PFMessage {
    //int sessionId; inherited from PFMessage
    int vehInFrontId;
}