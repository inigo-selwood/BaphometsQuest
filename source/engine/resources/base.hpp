#pragma once

#include <cstdint>
#include <ostream>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine::Resource {

using ID = std::uint64_t;

class Base {
  public:
    Base();
    virtual ~Base() = default;

    virtual std::string describe() const = 0;

    const std::string ID;

  protected:
    static std::string formatDescription(const ::YAML::Node &node);

  private:
    static std::string generateID();
};

inline std::ostream &operator<<(std::ostream &stream, const Base &resource) {
    return stream << resource.describe();
}

} // namespace Engine::Resource
