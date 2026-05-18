#include "yaml.hpp"

namespace Engine::Resource {

YAML::YAML(const std::string &path)
    : node(std::make_unique<::YAML::Node>(::YAML::LoadFile(path))),
      path(path) {}

std::string YAML::key(const std::string &path) {
    return "YAML:" + path;
}

std::string YAML::describe() const {
    ::YAML::Node name;
    name["type"] = "YAML";
    name["path"] = this->path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
