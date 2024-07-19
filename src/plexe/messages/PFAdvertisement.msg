import PlatooningBeacon;

packet PFAdvertisement extends PlatooningBeacon {
    int platoonId;
    bool isLeader;
    bool canBeLeader;
    double ccDesiredSpeed;
    double minToleratedSpeed;
    double maxToleratedSpeed;
    int currentPlatoonSize;
    int currentLane;
    int maneuverCapabilitiesCode;
    string nextCheckpoint;
    int negotiationChannel;
}
