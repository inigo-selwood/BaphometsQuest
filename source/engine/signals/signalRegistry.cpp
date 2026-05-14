#include "signalRegistry.hpp"

namespace Engine {

bool SignalRegistry::has(const std::string &name) const {
    return this->signals.contains(name);
}

SignalRegistry::Signal &
SignalRegistry::get(const std::string &name, const std::string &ownerName) {
    if(!this->has(name)) {
        throw std::runtime_error(
            "Unknown signal '" + name + "' on node '" + ownerName + "'."
        );
    }

    return this->signals.at(name);
}

const SignalRegistry::Signal &SignalRegistry::get(
    const std::string &name,
    const std::string &ownerName
) const {
    if(!this->has(name)) {
        throw std::runtime_error(
            "Unknown signal '" + name + "' on node '" + ownerName + "'."
        );
    }

    return this->signals.at(name);
}

} // namespace Engine
