package org.car2x.plexe.traffic;

import org.car2x.plexe.mobility.BTraCIBaseTrafficManager;

simple SimpleTrafficManager extends BTraCIBaseTrafficManager {
parameters:
    @class(plexe::SimpleTrafficManager);
    double insertionSpeed @unit("kmph") = default(90 kmph);
    string platooningVType = default("vtypecomm");
    string lane0 = default("");
    string lane1 = default("");
    string lane2 = default("");
    double p1startPos;
    double p2startPos;
    string route = default("E0");
    double vlength @unit("m") = default(4 m);
    double maxSpeed @unit("kmph") = default(130 kmph);
    double minSpeed @unit("kmph") = default(100 kmph);
}