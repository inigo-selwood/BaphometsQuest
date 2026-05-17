#include "manager.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

void Manager::clear() {
    for(const auto &resource : this->resources) {
        const std::string description = resource.second->describe();
        const ::YAML::Node details = ::YAML::Load(description);
        spdlog::debug(
            "freeing {}:\n{}",
            details["type"].as<std::string>("resource"),
            description
        );
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

    const std::string description = resource->second->describe();
    const ::YAML::Node details = ::YAML::Load(description);
    spdlog::debug(
        "freeing {}:\n{}",
        details["type"].as<std::string>("resource"),
        description
    );
    this->resources.erase(resource);
}

} // namespace Engine::Resource
