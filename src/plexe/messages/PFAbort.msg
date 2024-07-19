import PFMessage;

packet PFAbort extends PFMessage {
    //int sessionId; inherited from PFMessage
    string reason;
}