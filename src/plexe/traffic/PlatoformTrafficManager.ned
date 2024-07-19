package org.car2x.plexe.traffic;

import org.car2x.plexe.mobility.BTraCIBaseTrafficManager;

network PlatoformTrafficManager extends BTraCIBaseTrafficManager {
    parameters:
        // The insertionHeadwayTime indicates the minimum bumper-to-bumper
        // headway that the traffic insertion process should grant between
        // two consecutive cars inserted on the same lane
        double insertionHeadwayTime @unit("s");
        double minInsertionSpace @unit("m") = default(6m);
        double insertionSpeed @unit("kmph") = default(90 kmph);
        // communication-enbled vehicles penetration rate
        // 0 ==> no cars with radio, 1==> all cars have a radio and communicate
        double penetrationRate;
        // name of the modules to use for modelling, respectively,
        // - platooningVType ==> cars that can become  part of a platoon;
        // - noCommVType ==> cars without a radio interface;
        string platooningVType;
        string noCommVType;
        // number of lanes at the entrance of the Highway
        int entryLanes;
        bool trafficPreloading = default(true);

        double maxSpeed @unit("kmph");
        double minSpeed @unit("kmph");
        double stepSpeed @unit("kmph") = default(1 kmph);

        @class(plexe::PlatoformTrafficManager);

    submodules:
        entryProcesses[entryLanes]: PlatoformTrafficInjectionProcess{
        };

}
