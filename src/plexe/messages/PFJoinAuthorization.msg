import PFMessage;

packet PFJoinAuthorization extends PFMessage {
    //int sessionId; inherited from PFMessage
    int status;
    int platoonId;
    int manuverType;
}