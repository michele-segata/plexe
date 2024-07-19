package org.car2x.plexe.scenarios;

import org.car2x.plexe.scenarios.ManeuverScenario;

simple PFSimpleScenario extends ManeuverScenario
{
    parameters:
        @display("i=block/app2");
        @class(plexe::PFSimpleScenario);
        double insertionHeadwayTime @unit("s") = default(1.2 s);
        string platooningVType;

        double p1speed @unit("kmph") = default(100 kmph);
        double p2speed @unit("kmph") = default(110 kmph);
        string scenarioName;
        double startAdvertising @unit("s");
}
