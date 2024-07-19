package org.car2x.plexe.apps;

import org.car2x.plexe.apps.GeneralPlatooningApp;

module PFApp extends GeneralPlatooningApp
{
    parameters:
        @class(plexe::PFApp);

        int maxPlatoSize = default(25);
        int requiredValidAdvertisement = default(3);

        double maxAdvertiserDistance @unit("m") = default(100m);
        double minAdvertiserDistance @unit("m") = default(20m);

        int laneTolerance = default(1); //+/- laneTolerance requesterLaneIndex will be accepted
        double ccDesSpeedTolerance @unit("kmph") = default(10 kmph); //+/- ccDesSpeedTolerance advertised as min/maxToleratedCCDesSpeed
        int ccSpeedMinOverlap @unit("kmph") = default(5 kmph);

        double timeoutDuration @unit("s") = default(1 s);
        double gracePeriodAfterAbort @unit("s") = default(20 s);
        double gracePeriodAfterSuccess @unit("s") = default(5 s);
        double gracePeriodMaxRandomOffset = default(0.1);

        string vtype = default("vtypecomm");
        string seqdiag = default("");

        // for platoon-state stats
        @signal[platoonId_sig](type="int");
        @statistic[platoonId](source="platoonId_sig"; record=vector,stats?,timeavg?);

        @signal[platoonSize_sig](type="int");
        @statistic[platoonSize](source="platoonSize_sig"; record=vector,stats?,timeavg?);

        @signal[platoVarX_sig](type="double");
        @statistic[platoVarX](source="platoVarX_sig"; record=vector,stats?,timeavg?);

        @signal[platoleaderFlag_sig](type="bool");
        @statistic[platoleaderFlag](source="platoleaderFlag_sig"; record=vector,stats?,timeavg?);

        @signal[pfvehId_sig](type="long");
        @statistic[pfvehId](source="pfvehId_sig"; record=vector);
        
        
        // for pfsessions stats
        @signal[pfvehSessId_sig](type="long");
        @statistic[pfvehSessId](source="pfvehSessId_sig"; record=vector);

        @signal[pfoperation_sig](type="int");
        @statistic[pfoperation](source="pfoperation_sig"; record=vector);
        
        @signal[pfcoordId_sig](type="long");
        @statistic[pfcoordId](source="pfcoordId_sig"; record=vector);
		
		@signal[pfsessionId_sig](type="unsigned int");
        @statistic[pfsessionId](source="pfsessionId_sig"; record=vector);
        
        @signal[pfsessionStartTime_sig](type="double");
        @statistic[pfsession_startT](source="pfsessionStartTime_sig"; record=vector);

		@signal[pfsessionEndTime_sig](type="double");
        @statistic[pfsession_endT](source="pfsessionEndTime_sig"; record=vector);
        
        @signal[pfsessionStartX_sig](type="double");
        @statistic[pfsession_startX](source="pfsessionStartX_sig"; record=vector);

		@signal[pfsessionEndX_sig](type="double");
        @statistic[pfsession_endX](source="pfsessionEndX_sig"; record=vector);
		
    gates:
        input reqIn;
        input advIn;
}
