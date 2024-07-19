package org.car2x.plexe.scenarios;

import org.car2x.plexe.scenarios.ManeuverScenario;

simple PFScenario extends ManeuverScenario
{
    parameters:
        @display("i=block/app2");
        @class(plexe::PFScenario);
        double insertionHeadwayTime @unit("s");
        string platooningVType;
        
        string humstart = default("humstart");
        string cooldown = default("cooldown");
        string switchoff = default("switchoff");
}
