#include "Base.hpp"

namespace Engine::Resource {

std::string Base::formatDescription(const ::YAML::Node &node) {
    ::YAML::Emitter emitter;

    emitter << ::YAML::Block;
    emitter << node;

    return emitter.c_str();
}

} // namespace Engine::Resource
