#pragma once

#include <cstdint>
#include <ostream>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

using ID = std::uint64_t;

class Manager;

class Base {
  public:
    explicit Base(std::string name);
    virtual ~Base() = default;

    virtual std::string describe() const = 0;

    const std::string name;

  protected:
    static std::string formatDescription(const ::YAML::Node &node);
    static Engine::Resource::ID hashKey(const std::string &value);
};

inline std::ostream &operator<<(std::ostream &stream, const Base &resource) {
    return stream << resource.describe();
}

} // namespace Engine::Resource
