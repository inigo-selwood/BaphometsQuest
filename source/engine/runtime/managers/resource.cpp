#include "resource.hpp"

#include <spdlog/spdlog.h>

namespace Engine::Resource {

Manager::~Manager() {
    this->clear();
}

void Manager::clear() {
    for(const auto &resource : this->resources) {
        spdlog::debug("Freed {}", resource.second->ID);
    }

    this->resources.clear();
}

Base &Manager::get(ID id) {
    const auto resource = this->resources.find(id);

    if(resource == this->resources.end()) {
        throw std::runtime_error(
            "Resource " + std::to_string(id) + " does not exist."
        );
    }

    return *resource->second;
}

const Base &Manager::get(ID id) const {
    const auto resource = this->resources.find(id);

    if(resource == this->resources.end()) {
        throw std::runtime_error(
            "Resource " + std::to_string(id) + " does not exist."
        );
    }

    return *resource->second;
}

void Manager::remove(ID id) {
    const auto resource = this->resources.find(id);

    if(resource == this->resources.end()) {
        throw std::runtime_error(
            "Resource " + std::to_string(id) + " does not exist."
        );
    }

    spdlog::debug("Freed {}", resource->second->ID);
    this->resources.erase(resource);
}

} // namespace Engine::Resource
