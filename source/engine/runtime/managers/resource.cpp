#include "resource.hpp"

#include <spdlog/spdlog.h>

namespace Engine::Resource {

std::unique_ptr<Base>
ResourceLoader<TextTexture, SDL_Renderer *, ID, SDL_Color, std::string>::
    create(
        Manager &manager,
        SDL_Renderer *renderer,
        ID fontID,
        SDL_Color colour,
        const std::string &text
    ) {
    const Engine::Resource::Font &font =
        manager.get<Engine::Resource::Font>(fontID);

    return std::make_unique<Engine::Resource::TextTexture>(
        renderer,
        fontID,
        font,
        colour,
        text
    );
}

Manager::~Manager() {
    this->clear();
}

void Manager::clear() {
    for(const auto &resource : this->resources) {
        if(resource.second.resource == nullptr) {
            continue;
        }

        spdlog::debug("Freed {}", resource.second.resource->ID);
    }

    this->resources.clear();
    this->ids.clear();
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
        resource->second.resource = resource->second.factory();
        const std::string description = resource->second.resource->describe();
        spdlog::debug(
            "Loaded {}:\n{}",
            resource->second.resource->ID,
            Logger::indentPayload(description)
        );
    }

    return *resource->second.resource;
}

const Base &Manager::get(ID id) const {
    return const_cast<Manager *>(this)->get(id);
}

void Manager::purgeExpired() {
    const Clock::time_point now = Clock::now();

    for(auto &[_, resource] : this->resources) {
        if(resource.resource == nullptr) {
            continue;
        }

        if(now - resource.lastAccessedAt < EXPIRY) {
            continue;
        }

        spdlog::debug("Freed {}", resource.resource->ID);
        resource.resource.reset();
    }
}

} // namespace Engine::Resource
