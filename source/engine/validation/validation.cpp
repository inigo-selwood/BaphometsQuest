#include "validation.hpp"

namespace Engine {

namespace Validation {

bool requireBool(
    const YAML::Node &parent,
    const std::string &name,
    const std::string &context
) {
    const YAML::Node node = parent[name];

    if(!node || !node.IsScalar()) {
        throw std::runtime_error(
            context + " is missing required value '" + name + "'."
        );
    }

    try {
        return node.as<bool>();
    } catch(const YAML::Exception &) {
        throw std::runtime_error(
            context + " value '" + name + "' must be true or false."
        );
    }
}

YAML::Node requireMap(
    const YAML::Node &parent,
    const std::string &name,
    const std::string &context
) {
    const YAML::Node node = parent[name];

    if(!node || !node.IsMap()) {
        throw std::runtime_error(
            context + " is missing required map '" + name + "'."
        );
    }

    return node;
}

std::string requireString(
    const YAML::Node &parent,
    const std::string &name,
    const std::string &context
) {
    const YAML::Node node = parent[name];

    if(!node || !node.IsScalar()) {
        throw std::runtime_error(
            context + " is missing required value '" + name + "'."
        );
    }

    const auto value = node.as<std::string>();

    if(value.empty()) {
        throw std::runtime_error(
            context + " value '" + name + "' must not be empty."
        );
    }

    return value;
}

} // namespace Validation

} // namespace Engine
