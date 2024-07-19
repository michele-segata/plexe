import PlatooningBeacon;

packet PlatoformAdvertisingMessage extends PlatooningBeacon {
    // this car accept to become a platoonLeader if needed
    //bool canBeLeader;
    bool isLeader;
    double cruiseSpeed;
    int currentPlatoonSize;
    int platoonId;
    // next "Google-Map alike turn point coordinates" so that who
    // wants to leave earlier can ignore this advertisment
    // string nextCheckpoint;
}
