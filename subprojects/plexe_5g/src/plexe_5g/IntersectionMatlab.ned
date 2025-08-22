package org.car2x.plexe.subprojects.plexe_5g;

simple IntersectionMatlab
{
    parameters:
        double alpha = default(.5);
        string matlabSessionName;
        
    @class(plexe::IntersectionMatlab);
}