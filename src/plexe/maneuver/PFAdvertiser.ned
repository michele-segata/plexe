package org.car2x.plexe.maneuver;
import org.car2x.plexe.maneuver.PFModule;

simple PFAdvertiser extends PFModule
{
parameters:
    @class(plexe::PFAdvertiser);

    double interMergeMandatoryTimeGap @unit("s") = default(3.0s);

    int maneuverCapabilities = default(1234); //fake code meaning "any maneuver"
}