#pragma once

#include <ostream>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

class Base {
  public:
    Base() = default;
    Base(const Base &) = delete;
    Base &operator=(const Base &) = delete;
    Base(Base &&) = default;
    Base &operator=(Base &&) = default;

    virtual ~Base() = default;

    virtual std::string describe() const = 0;

  protected:
    static std::string formatDescription(const ::YAML::Node &node);
};

inline std::ostream &operator<<(std::ostream &stream, const Base &resource) {
    return stream << resource.describe();
}

} // namespace Engine::Resource
