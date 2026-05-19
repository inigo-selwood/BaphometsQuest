#pragma once

#include "../base.hpp"

#include <chrono>
#include <memory>
#include <string>

#include <tinyxml2.h>

namespace Engine::Resource {

/** Parsed XML document resource */
class XML : public Engine::Resource::Base {
  private:
    static std::unique_ptr<tinyxml2::XMLDocument>
    load(const std::string &path);

  public:
    explicit XML(const std::string &path);

    /** Time a live cached XML document can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{300};

    /** Create a cache-owned XML resource */
    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<XML>(path);
    }

    /** Return the stable cache ID for an XML path */
    static Engine::Resource::ID key(const std::string &path);

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Owned parsed XML document */
    const std::unique_ptr<tinyxml2::XMLDocument> document;

    /** Source XML path */
    const std::string path;
};

} // namespace Engine::Resource
