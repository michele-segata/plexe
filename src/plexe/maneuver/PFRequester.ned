package org.car2x.plexe.maneuver;
import org.car2x.plexe.maneuver.PFModule;

simple PFRequester extends PFModule
{
parameters:
    @class(plexe::PFRequester);
    int maneuverCapabilities = default(1234); //fake code meaning "any maneuver"
    double maxPreparationTime @unit("s") = default(20 s);
}