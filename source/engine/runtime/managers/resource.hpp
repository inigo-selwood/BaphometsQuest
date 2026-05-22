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

/** Default resource construction adapter used by the cache manager */
template <typename ResourceType, typename... Arguments> struct ResourceLoader {
    static Engine::Resource::ID key(const Arguments &...arguments) {
        return ResourceType::key(arguments...);
    }

    static std::unique_ptr<Base>
    create(Manager &manager, const Arguments &...arguments) {
        return ResourceType::create(manager, arguments...);
    }
};

/** Timed resource cache that preserves stable IDs after resources expire */
class Manager {
  public:
    ~Manager();

    /** Destroy all cached resources and forget their IDs */
    void clear();

    /** Return a cached resource, reconstructing it first if it expired */
    Base &get(ID id);

    /** Return a cached resource, reconstructing it first if it expired */
    const Base &get(ID id) const;

    /** Destroy one live cached resource while keeping its stable ID */
    void unload(ID id);

    /** Destroy live cached resources of one type while keeping stable IDs */
    template <typename ResourceType> void unloadAll() {
        static_assert(
            std::is_base_of_v<Base, ResourceType>,
            "ResourceType must inherit from Engine::Resource::Base."
        );

        for(auto &[id, resource] : this->resources) {
            if(resource.resource == nullptr) {
                continue;
            }

            if(dynamic_cast<ResourceType *>(resource.resource.get())
                == nullptr) {
                continue;
            }

            spdlog::debug("Freed {:016x} ({})", id, resource.resource->name);
            resource.resource.reset();
        }
    }

    /** Destroy live cached resources except the listed types */
    template <typename... ResourceTypes> void unloadAllExcept() {
        static_assert(
            (std::is_base_of_v<Base, ResourceTypes> && ...),
            "ResourceTypes must inherit from Engine::Resource::Base."
        );

        for(auto &[id, resource] : this->resources) {
            if(resource.resource == nullptr) {
                continue;
            }

            const bool shouldKeep =
                (
                    ...
                    || (dynamic_cast<ResourceTypes *>(resource.resource.get())
                        != nullptr)
                );

            if(shouldKeep) {
                continue;
            }

            spdlog::debug("Freed {:016x} ({})", id, resource.resource->name);
            resource.resource.reset();
        }
    }

    /** Unload live resources that have exceeded their type-specific TTL */
    void purgeExpired();

    /**
     * Return the stable ID for a resource identity, loading it on first use
     *
     * ResourceType owns identity, construction, and TTL policy through key(),
     * create(), and TTL
     */
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

    /** Cache ledger entry kept even when the live resource has expired */
    struct ResourceEntry {
        std::unique_ptr<Base> resource;
        std::function<std::unique_ptr<Base>()> factory;
        std::chrono::seconds ttl;
        Clock::time_point lastAccessedAt;
    };

    std::unordered_map<ID, ResourceEntry> resources;
};

} // namespace Engine::Resource
