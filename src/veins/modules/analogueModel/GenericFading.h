//
// Copright (c) 2013-2015 Michele Segata <segata@ccs-labs.org>
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

#ifndef GENERICFADING_H_
#define GENERICFADING_H_

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/phyLayer/Mapping.h"

class GenericFadingMapping;

/**
 * @brief Implements a fading model that include different fading distributions.
 *
 * An example config.xml for this AnalogueModel can be the following:
 * @verbatim
	<AnalogueModel type="GenericFading">

		<!-- Distribution type. For now, only gaussian and nakagami are implemented -->
		<parameter name="distribution" type="string" value="gaussian">

		<!-- Distribution parameter 1. For the gaussian distribution this is the standard deviation -->
		<!-- Distribution parameter 1. For the nakagami distribution this is the m value -->
		<parameter name="param1" type="double" value="1"/>

	</AnalogueModel>
   @endverbatim
 *
 * @ingroup analogueModels
 * @author Michele Segata <segata@ccs-labs.org>
 */
class MIXIM_API GenericFading: public AnalogueModel {
protected:
	friend class GenericFadingMapping;

	/**
	 * Distributions that can be selected. For now, only gaussian and nakagami are implemented
	 */
	enum FADING_DISTRIBUTION {
		GAUSSIAN = 0,
		RAYLEIGH = 1,
		RICEAN = 2,
		NAKAGAMI = 3
	};

	/**
	 * @brief Type of distribution used to generate values
	 **/
	enum FADING_DISTRIBUTION distribution;

	/** @brief Parameters for the gaussian distribution */
	double stdDev;
	/** @brief Parameters for the nakagami-m distribution */
	double nakagamiM;

public:
	/**
	 * @brief Instantiates generic fading using the selected distribution
	 * and the given parameters.
	 *
	 * TODO: extend parameters for different distributions
	 *
	 * \param distribution string defining the distribution to be used. This can be
	 * 'gaussian', 'rayleigh', 'ricean', or 'nakagami'. For now, only gaussian (or log
	 * normal) and nakagami are implemented
	 * \param param1 for now, stddev of gaussian distribution
	 */
	GenericFading(std::string distribution, double param1);
	virtual ~GenericFading();

	virtual void filterSignal(AirFrame *, const Coord&, const Coord&);
};

/**
 * @brief Mapping used to represent attenuation of a generic fading. A distribution
 * with parameters can be selected
 *
 * @ingroup analogueModels
 * @ingroup mapping
 */
class MIXIM_API GenericFadingMapping: public SimpleConstMapping {
protected:
	static DimensionSet dimensions;

	/** @brief Pointer to the model.*/
	GenericFading* model;

	/** @brief the attenuation factor computed.*/
	double attenuationFactor;

public:
	/**
	 * @brief Takes the model, two hosts and
	 * the interval in which to create key entries.
	 */
	GenericFadingMapping(GenericFading* model,
	                     const Argument& start,
	                     const Argument& end):
		SimpleConstMapping(dimensions, start, end), model(model) {
		switch(model->distribution) {
			case GenericFading::GAUSSIAN:

				attenuationFactor = 1 / (pow(10.0, normal(0, model->stdDev) / 10.0));

				break;
			case GenericFading::NAKAGAMI:
				//the attenuation factor for a nakagami distribution can be obtained in several
				//ways from the gamma distribution. to compute the nakagami-attenuated power,
				//the ns-3 implementation uses gamma(m, txPowerW / m), while the INET framework
				//uses gamma(m, avgRxPowerW / m). in this case, we simply compute the attenuation
				//factor using gamma(m, 1/m)
				attenuationFactor = gamma_d(model->nakagamiM, 1 / model->nakagamiM);
				break;
			default:
				ASSERT2(0, "invalid distribution type set for GenericFading");
				break;
		}
	}

	/**
	 * Return the attenuation factor to be applied. For example, if an attenuation of 30 dB
	 * is required, this function should return 1/10^(30/10) = 0.001. What the simulator then does is
	 * to multiply the signal power by this factor. For example, if the signal power is 20 dBm,
	 * (100 mW) then mixim computes:
	 *
	 * att.signal = signal * factor = 100 mW * 0.001 = 0.1 mW = -10 dBm
	 *
	 * which is exactly 20 dBm - 30 dB
	 */
	virtual double getValue(const Argument& pos) const;

	/**
	 * @brief creates a clone of this mapping.
	 *
	 * This method has to be implemented by every subclass.
	 * But most time the implementation will look like the
	 * implementation of this method (except of the class name).
	 */
	ConstMapping* constClone() const {
		return new GenericFadingMapping(*this);
	}
};

#endif /* GENERICFADING_H_ */
