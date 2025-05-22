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

#include "IDIPAddressHandler.h"

Define_Module(IDIPAddressHandler);

inet::L3Address IDIPAddressHandler::IP_NOT_FOUND = inet::L3Address("0.0.0.0");

void IDIPAddressHandler::registerIDIPMapping(int vehicleId, inet::L3Address ip)
{
    EV << "IDIPAddressHandler: mapping vehicle id " << vehicleId << " to IP address " << ip << endl;
    IDIPmap[vehicleId] = ip;
}

inet::L3Address IDIPAddressHandler::getIpAddress(int vehicleId)
{
    auto search = IDIPmap.find(vehicleId);
    if (search == IDIPmap.end())
        return IP_NOT_FOUND;
    else
        return search->second;
}
int IDIPAddressHandler::getVehicleId(inet::L3Address ip)
{
    for (auto const& el : IDIPmap) {
        if (el.second == ip)
            return el.first;
    }
    return -1;
}
