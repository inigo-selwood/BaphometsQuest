#pragma once

#include "../base.hpp"

#include <chrono>
#include <memory>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

/** Parsed YAML document resource */
class YAML : public Engine::Resource::Base {
  public:
    explicit YAML(const std::string &path);

    /** Time a live cached YAML document can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{300};

    /** Create a cache-owned YAML resource */
    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<YAML>(path);
    }

    /** Return the stable cache ID for a YAML path */
    static Engine::Resource::ID key(const std::string &path);

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Owned parsed YAML node */
    const std::unique_ptr<::YAML::Node> node;

    /** Source YAML path */
    const std::string path;
};

} // namespace Engine::Resource
