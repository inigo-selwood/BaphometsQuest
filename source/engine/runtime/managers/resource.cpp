#include "resource.hpp"

#include <spdlog/spdlog.h>

namespace Engine::Resource {

Manager::~Manager() {
    this->clear();
}

void Manager::clear() {
    for(const auto &[id, resource] : this->resources) {
        if(resource.resource == nullptr) {
            continue;
        }

        spdlog::debug("Freed {:016x} ({})", id, resource.resource->name);
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

    resource->second.lastAccessedAt = Clock::now();

    if(resource->second.resource == nullptr) {
        // Expired entries keep their factory so stable IDs can become live
        // again
        resource->second.resource = resource->second.factory();
        const std::string description = resource->second.resource->describe();
        spdlog::debug(
            "Loaded {:016x}:\n{}",
            id,
            Logger::indentPayload(description)
        );
    }

    return *resource->second.resource;
}

const Base &Manager::get(ID id) const {
    return const_cast<Manager *>(this)->get(id);
}

void Manager::unload(ID id) {
    const auto resource = this->resources.find(id);

    if(resource == this->resources.end()) {
        throw std::runtime_error(
            "Resource " + std::to_string(id) + " does not exist."
        );
    }

    if(resource->second.resource == nullptr) {
        return;
    }

    spdlog::debug("Freed {:016x} ({})", id, resource->second.resource->name);
    resource->second.resource.reset();
}

void Manager::purgeExpired() {
    const Clock::time_point now = Clock::now();

    for(auto &[id, resource] : this->resources) {
        if(resource.resource == nullptr) {
            continue;
        }

        if(resource.ttl.count() < 0) {
            continue;
        }

        if(now - resource.lastAccessedAt < resource.ttl) {
            continue;
        }

        spdlog::debug("Freed {:016x} ({})", id, resource.resource->name);
        resource.resource.reset();
    }
}

} // namespace Engine::Resource
