#pragma once

#include "node.hpp"

#include "../format/format.hpp"
#include "../parse/parse.hpp"

#include <SDL.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <utility>

#include <spdlog/spdlog.h>

namespace Engine {

template <typename ValueType> struct PropertyParser {
    static ValueType parse(const std::string &, const std::string &) = delete;
};

template <> struct PropertyParser<bool> {
    static bool parse(const std::string &value, const std::string &name) {
        return Parse::boolean(value, name);
    }
};

template <> struct PropertyParser<float> {
    static float parse(const std::string &value, const std::string &name) {
        return Parse::floating(value, name);
    }
};

template <> struct PropertyParser<int> {
    static int parse(const std::string &value, const std::string &name) {
        return Parse::integer(value, name);
    }
};

template <> struct PropertyParser<SDL_Color> {
    static SDL_Color parse(const std::string &value, const std::string &name) {
        return Parse::colour(value, name);
    }
};

template <> struct PropertyParser<SDL_Point> {
    static SDL_Point parse(const std::string &value, const std::string &name) {
        return Parse::point(value, name);
    }
};

template <> struct PropertyParser<SDL_Rect> {
    static SDL_Rect parse(const std::string &value, const std::string &name) {
        return Parse::rect(value, name);
    }
};

template <> struct PropertyParser<std::string> {
    static std::string parse(const std::string &value, const std::string &) {
        return value;
    }
};

template <typename ValueType>
std::string propertyValueToString(const ValueType &value) {
    return Format::value(value);
}

template <> inline std::string propertyValueToString<bool>(const bool &value) {
    return Format::boolean(value);
}

template <>
inline std::string propertyValueToString<SDL_Color>(const SDL_Color &value) {
    return Format::colour(value);
}

template <>
inline std::string propertyValueToString<SDL_Point>(const SDL_Point &value) {
    return Format::point(value);
}

template <>
inline std::string propertyValueToString<SDL_Rect>(const SDL_Rect &value) {
    return Format::rect(value);
}

struct Node::PropertyBindingBase {
    PropertyBindingBase(
        std::string propertyName,
        std::string propertyOwner,
        std::type_index propertyType
    )
        : name(std::move(propertyName)), owner(std::move(propertyOwner)),
          type(propertyType) {}

    virtual ~PropertyBindingBase() = default;

    virtual void setFromString(const std::string &value) = 0;
    virtual std::string valueToString() const = 0;

    std::string name;
    std::string owner;
    std::type_index type;
};

template <typename ValueType>
struct Node::PropertyBinding final : PropertyBindingBase {
    PropertyBinding(
        const std::string &propertyName,
        const std::string &propertyOwner,
        std::function<ValueType(const std::string &, const std::string &)>
            propertyParser,
        std::function<ValueType()> propertyGetter,
        std::function<void(const ValueType &)> propertySetter,
        std::function<std::string(const ValueType &)> propertyFormatter
    )
        : PropertyBindingBase(
              propertyName,
              propertyOwner,
              std::type_index(typeid(ValueType))
          ),
          formatter(std::move(propertyFormatter)),
          getter(std::move(propertyGetter)), parser(std::move(propertyParser)),
          setter(std::move(propertySetter)) {}

    void setFromString(const std::string &value) override {
        this->setter(this->parser(value, this->name));
    }

    std::string valueToString() const override {
        return this->formatter(this->getter());
    }

    std::function<std::string(const ValueType &)> formatter;
    std::function<ValueType()> getter;
    std::function<ValueType(const std::string &, const std::string &)> parser;
    std::function<void(const ValueType &)> setter;
};

template <typename ValueType>
ValueType Node::getProperty(const std::string &name) const {
    const auto &property = this->getPropertyBinding(name);

    if(property.type != std::type_index(typeid(ValueType))) {
        throw std::runtime_error(
            "Property '" + name + "' on node '" + this->getName()
            + "' has an unexpected type."
        );
    }

    const auto &typedProperty =
        static_cast<const PropertyBinding<ValueType> &>(property);
    return typedProperty.getter();
}

template <typename ValueType>
void Node::setProperty(const std::string &name, const ValueType &value) {
    auto &property = this->getPropertyBinding(name);

    if(property.type != std::type_index(typeid(ValueType))) {
        throw std::runtime_error(
            "Property '" + name + "' on node '" + this->getName()
            + "' has an unexpected type."
        );
    }

    auto &typedProperty = static_cast<PropertyBinding<ValueType> &>(property);
    typedProperty.setter(value);
    spdlog::trace(
        "set property {}.{}: to {}",
        this->getName(),
        name,
        typedProperty.valueToString()
    );
}

template <typename ValueType>
void Node::registerProperty(
    const std::string &name,
    const std::string &owner,
    std::function<ValueType(const std::string &, const std::string &)> parser,
    std::function<ValueType()> getter,
    std::function<void(const ValueType &)> setter,
    std::function<std::string(const ValueType &)> formatter
) {
    if(this->properties.contains(name)) {
        throw std::runtime_error(
            "Property '" + name + "' is already registered on node '"
            + this->getName() + "'."
        );
    }

    this->properties.emplace(
        name,
        std::make_unique<PropertyBinding<ValueType>>(
            name,
            owner,
            std::move(parser),
            std::move(getter),
            std::move(setter),
            std::move(formatter)
        )
    );
}

template <typename ValueType>
void Node::registerProperty(
    const std::string &name,
    const std::string &owner,
    std::function<ValueType()> getter,
    std::function<void(const ValueType &)> setter
) {
    this->registerProperty<ValueType>(
        name,
        owner,
        PropertyParser<ValueType>::parse,
        std::move(getter),
        std::move(setter),
        propertyValueToString<ValueType>
    );
}

template <typename ValueType>
void Node::registerProperty(
    const std::string &name,
    const std::string &owner,
    std::function<ValueType()> getter,
    std::function<void(const ValueType &)> setter,
    std::function<std::string(const ValueType &)> formatter
) {
    this->registerProperty<ValueType>(
        name,
        owner,
        PropertyParser<ValueType>::parse,
        std::move(getter),
        std::move(setter),
        std::move(formatter)
    );
}

} // namespace Engine
