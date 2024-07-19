import PlatoformMessage;

packet PlatoformRequest extends PlatoformMessage {
    int requesterLane;
    int requestedPlatoonId;
    string requsterFrontVehId;
}