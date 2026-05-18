#include "base.hpp"

#include <functional>
#include <utility>

namespace Engine::Resource {

Base::Base(std::string name) : name(std::move(name)) {}

std::string Base::formatDescription(const ::YAML::Node &node) {
    ::YAML::Emitter emitter;

    emitter << ::YAML::Block;
    emitter << node;

    return emitter.c_str();
}

Engine::Resource::ID Base::hashKey(const std::string &value) {
    return static_cast<Engine::Resource::ID>(std::hash<std::string>{}(value));
}

} // namespace Engine::Resource
