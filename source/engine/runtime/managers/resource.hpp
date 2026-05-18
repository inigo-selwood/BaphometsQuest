#pragma once

#include "../../../core/logger.hpp"
#include "../../resources/base.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <spdlog/spdlog.h>

namespace Engine::Resource {

template <typename ResourceType, typename... Arguments> struct ResourceLoader {
    static Engine::Resource::ID key(const Arguments &...arguments) {
        return ResourceType::key(arguments...);
    }

    static std::unique_ptr<Base>
    create(Manager &manager, const Arguments &...arguments) {
        return ResourceType::create(manager, arguments...);
    }
};

class Manager {
  public:
    ~Manager();

    void clear();
    Base &get(ID id);
    const Base &get(ID id) const;
    void purgeExpired();

    template <typename ResourceType, typename... Arguments>
    ID load(Arguments &&...arguments) {
        static_assert(
            std::is_base_of_v<Base, ResourceType>,
            "ResourceType must inherit from Engine::Resource::Base."
        );

        using Loader =
            ResourceLoader<ResourceType, std::decay_t<Arguments>...>;
        using StoredArguments = std::tuple<std::decay_t<Arguments>...>;

        const Engine::Resource::ID id = Loader::key(arguments...);
        const auto existingResource = this->resources.find(id);

        if(existingResource != this->resources.end()) {
            existingResource->second.lastAccessedAt = Clock::now();

            return id;
        }

        StoredArguments storedArguments(std::forward<Arguments>(arguments)...);

        auto factory = [this, storedArguments]() mutable {
            return std::apply(
                [this](const auto &...arguments) {
                    return Loader::create(*this, arguments...);
                },
                storedArguments
            );
        };

        auto resource = factory();
        ResourceEntry entry{
            std::move(resource),
            factory,
            ResourceType::TTL,
            Clock::now(),
        };

        this->resources.emplace(id, std::move(entry));

        const Base &loadedResource = *this->resources.at(id).resource;
        const std::string description = loadedResource.describe();
        spdlog::debug(
            "Loaded {:016x}:\n{}",
            id,
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
    using Clock = std::chrono::steady_clock;

    struct ResourceEntry {
        std::unique_ptr<Base> resource;
        std::function<std::unique_ptr<Base>()> factory;
        std::chrono::seconds ttl;
        Clock::time_point lastAccessedAt;
    };

    std::unordered_map<ID, ResourceEntry> resources;
};

} // namespace Engine::Resource
