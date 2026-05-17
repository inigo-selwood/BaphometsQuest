#include "yaml.hpp"

namespace Engine::Resource {

YAML::YAML(const std::string &path)
    : Node(std::make_unique<::YAML::Node>(::YAML::LoadFile(path))),
      Path(path) {}

std::string YAML::describe() const {
    ::YAML::Node name;
    name["type"] = "YAML";
    name["path"] = this->Path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
