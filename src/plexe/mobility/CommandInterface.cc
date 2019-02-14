#include "CommandInterface.h"

#include <veins/modules/mobility/traci/TraCICommandInterface.h>
#include <veins/modules/mobility/traci/TraCIConnection.h>

namespace plexe {
namespace traci {

CommandInterface::CommandInterface(cComponent* owner, Veins::TraCICommandInterface* veinsCommandInterface, Veins::TraCIConnection* connection)
    : HasLogProxy(owner)
    , veinsCommandInterface(veinsCommandInterface)
    , connection(connection)
{}

} // namespace traci
} // namespace plexe
