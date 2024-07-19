import PFMessage;

packet PFUpdate extends PFMessage {
    //int sessionId; inherited from PFMessage
    int platoonId;
    int platoonLeaderId;
}