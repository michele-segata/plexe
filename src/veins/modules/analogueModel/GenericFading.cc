//
// Copyright (c) 2013-2015 Michele Segata <segata@ccs-labs.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "veins/modules/analogueModel/GenericFading.h"

#include "veins/base/phyLayer/Signal_.h"
#include "veins/base/messages/AirFrame_m.h"

DimensionSet GenericFadingMapping::dimensions(Dimension::time);

double GenericFadingMapping::getValue(const Argument& pos) const {
	return attenuationFactor;
}


GenericFading::GenericFading(std::string distribution, double param1) {

	if (strcasecmp(distribution.c_str(), "gaussian") == 0) {
		this->distribution = GAUSSIAN;
		stdDev = param1;
	}
	else if (strcasecmp(distribution.c_str(), "nakagami") == 0) {
		this->distribution = NAKAGAMI;
		nakagamiM = param1;
	}
	else {
		//TODO: implement other distributions
		ASSERT2(0, "selected distribution is unknown or not implemented yet");
	}

}

GenericFading::~GenericFading() {}

void GenericFading::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos) {
	Signal& signal = frame->getSignal();
	signal.addAttenuation(new GenericFadingMapping(this,
	                                               Argument(signal.getReceptionStart()),
	                                               Argument(signal.getReceptionEnd())));
}
