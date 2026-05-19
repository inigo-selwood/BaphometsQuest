#include "yaml.hpp"

#include "../../utils/format.hpp"

namespace Engine::Resource {

YAML::YAML(const std::string &path)
    : Base("yaml"),
      node(std::make_unique<::YAML::Node>(::YAML::LoadFile(path))),
      path(path) {}

Engine::Resource::ID YAML::key(const std::string &path) {
    return hashKey("YAML:" + path);
}

std::string YAML::describe() const {
    ::YAML::Node name;
    name["type"] = "YAML";
    name["path"] = Engine::Format::path(this->path);

    return this->formatDescription(name);
}

} // namespace Engine::Resource
