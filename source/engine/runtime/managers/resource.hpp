#pragma once

#include "../../../logger.hpp"
#include "../../resources/base.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <spdlog/spdlog.h>

namespace Engine::Resource {

using ID = std::uint64_t;

class Manager {
  public:
    ~Manager();

    void clear();
    Base &get(ID id);
    const Base &get(ID id) const;
    void remove(ID id);

    template <typename ResourceType, typename... Arguments>
    ID load(Arguments &&...arguments) {
        static_assert(
            std::is_base_of_v<Base, ResourceType>,
            "ResourceType must inherit from Engine::Resource::Base."
        );

        const ID id = this->nextResourceID++;

        auto resource = std::make_unique<ResourceType>(
            std::forward<Arguments>(arguments)...
        );

        this->resources.emplace(id, std::move(resource));

        const Base &loadedResource = *this->resources.at(id);
        const std::string description = loadedResource.describe();
        spdlog::debug(
            "Loaded {}:\n{}",
            loadedResource.ID,
            Logger::indentPayload(description)
        );

        return id;
    }

    template <typename ResourceType> ResourceType &get(ID id) {
        static_assert(
            std::is_base_of_v<Base, ResourceType>,
            "ResourceType must inherit from Engine::Resource::Base."
        );

        Base &resource = this->get(id);
        ResourceType *typedResource = dynamic_cast<ResourceType *>(&resource);

        if(!typedResource) {
            throw std::runtime_error(
                "Resource " + std::to_string(id)
                + " does not match the requested type."
            );
        }

        return *typedResource;
    }

    template <typename ResourceType> const ResourceType &get(ID id) const {
        static_assert(
            std::is_base_of_v<Base, ResourceType>,
            "ResourceType must inherit from Engine::Resource::Base."
        );

        const Base &resource = this->get(id);
        const ResourceType *typedResource =
            dynamic_cast<const ResourceType *>(&resource);

        if(!typedResource) {
            throw std::runtime_error(
                "Resource " + std::to_string(id)
                + " does not match the requested type."
            );
        }

        return *typedResource;
    }

  private:
    std::unordered_map<ID, std::unique_ptr<Base>> resources;
    ID nextResourceID = 1;
};

} // namespace Engine::Resource
