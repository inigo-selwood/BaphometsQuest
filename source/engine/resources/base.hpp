#pragma once

#include <cstdint>
#include <ostream>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

using ID = std::uint64_t;

class Manager;

/** Common interface for resources stored in the runtime cache */
class Base {
  public:
    explicit Base(std::string name);
    virtual ~Base() = default;

    /** Return YAML-formatted details for logging */
    virtual std::string describe() const = 0;

    /** Stable resource type name used in logs */
    const std::string name;

  protected:
    /** Convert a YAML node into the standard resource detail block */
    static std::string formatDescription(const ::YAML::Node &node);

    /** Convert deterministic resource identity text into a cache ID */
    static Engine::Resource::ID hashKey(const std::string &value);
};

inline std::ostream &operator<<(std::ostream &stream, const Base &resource) {
    return stream << resource.describe();
}

} // namespace Engine::Resource
