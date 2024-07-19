package org.car2x.plexe.traffic;

simple PlatoformTrafficInjectionProcess {
    parameters:
        double vehicleRate @unit("vpmin");
        double insertionDelay @unit("s");

        @class(plexe::PlatoformTrafficInjectionProcess);
}
