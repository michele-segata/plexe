#pragma once

#include <plexe/plexe.h>

#include <veins/modules/utility/HasLogProxy.h>

namespace Veins {
class TraCICommandInterface;
class TraCIConnection;
}

namespace plexe {
namespace traci {

class CommandInterface : public Veins::HasLogProxy {
public:
    CommandInterface(cComponent* owner, Veins::TraCICommandInterface* commandInterface, Veins::TraCIConnection* connection);
private:
    Veins::TraCICommandInterface* veinsCommandInterface;
    Veins::TraCIConnection* connection;
};

} // namespace traci
} // namespace plexe
