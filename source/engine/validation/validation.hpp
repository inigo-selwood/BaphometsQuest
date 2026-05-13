#pragma once

#include <stdexcept>
#include <string>

#include <yaml-cpp/yaml.h>

namespace Engine {

namespace Validation {

/** Return a required YAML boolean value. */
bool requireBool(
    const YAML::Node &parent,
    const std::string &name,
    const std::string &context
);

/** Return a required YAML mapping child node. */
YAML::Node requireMap(
    const YAML::Node &parent,
    const std::string &name,
    const std::string &context
);

/** Return a required positive YAML scalar value. */
template <typename ValueType>
ValueType requirePositiveValue(
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

    const auto value = node.as<ValueType>();

    if(value <= 0) {
        throw std::runtime_error(
            context + " value '" + name + "' must be greater than 0."
        );
    }

    return value;
}

/** Return a required non-empty YAML string value. */
std::string requireString(
    const YAML::Node &parent,
    const std::string &name,
    const std::string &context
);

} // namespace Validation

} // namespace Engine
