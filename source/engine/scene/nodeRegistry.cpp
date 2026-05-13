#include "nodeRegistry.hpp"

#include <stdexcept>
#include <utility>

namespace Engine {

NodeRegistry &NodeRegistry::getInstance() {
    static NodeRegistry registry;
    return registry;
}

std::unique_ptr<Node> NodeRegistry::create(const std::string &name) const {
    if(!this->factories.contains(name)) {
        throw std::runtime_error("Unknown node type: " + name);
    }

    spdlog::debug("Creating node '{}'.", name);

    auto node = this->factories.at(name)();
    node->nodeTypeName = name;

    return node;
}

void NodeRegistry::registerType(const std::string &name, NodeFactory factory) {
    spdlog::debug("Registering node '{}'.", name);
    this->factories[name] = std::move(factory);
}

} // namespace Engine
