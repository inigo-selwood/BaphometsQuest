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
    static Engine::Resource::Key key(const Arguments &...arguments) {
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

        const Engine::Resource::Key key = Loader::key(arguments...);
        const auto existingResource = this->ids.find(key);

        if(existingResource != this->ids.end()) {
            ResourceEntry &entry =
                this->resources.at(existingResource->second);
            entry.lastAccessedAt = Clock::now();

            return existingResource->second;
        }

        const ID id = this->nextResourceID++;
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
            key,
            std::move(resource),
            factory,
            ResourceType::TTL,
            Clock::now(),
        };

        this->resources.emplace(id, std::move(entry));
        this->ids.emplace(key, id);

        const Base &loadedResource = *this->resources.at(id).resource;
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
    using Clock = std::chrono::steady_clock;

    struct ResourceEntry {
        Engine::Resource::Key key;
        std::unique_ptr<Base> resource;
        std::function<std::unique_ptr<Base>()> factory;
        std::chrono::seconds ttl;
        Clock::time_point lastAccessedAt;
    };

    std::unordered_map<ID, ResourceEntry> resources;
    std::unordered_map<Engine::Resource::Key, ID> ids;
    ID nextResourceID = 1;
};

} // namespace Engine::Resource
