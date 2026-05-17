#include "Manager.hpp"

namespace Engine::Resource {

void Manager::clear() {
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
    const std::size_t removed = this->resources.erase(id);

    if(removed == 0) {
        throw std::runtime_error(
            "Resource " + std::to_string(id) + " does not exist."
        );
    }
}

} // namespace Engine::Resource
