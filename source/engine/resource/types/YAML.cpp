#include "YAML.hpp"

namespace Engine::Resource {

YAML::YAML(const std::string &path)
    : node(std::make_unique<::YAML::Node>(::YAML::LoadFile(path))),
      path(path) {}

::YAML::Node &YAML::getNode() const {
    return *this->node;
}

std::string YAML::describe() const {
    ::YAML::Node name;
    name["type"] = "YAML";
    name["path"] = this->path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
