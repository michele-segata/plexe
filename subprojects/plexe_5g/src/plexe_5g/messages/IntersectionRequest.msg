import RsuMessage; 

packet IntersectionRequest extends RsuMessage{
    int platoonId;
    int leaderId;
    double distance;
    double speed;
    double length;
    double vehCount;
    string roadId;
    double reqTime;
}