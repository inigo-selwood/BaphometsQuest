#pragma once

#include <SDL.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Engine::Format {

/** Convert an SDL colour to #rrggbbaa text */
inline std::string colour(const SDL_Color &colour) {
    std::ostringstream stream;

    stream << "#";
    stream << std::hex << std::nouppercase << std::setfill('0');
    stream << std::setw(2) << static_cast<int>(colour.r);
    stream << std::setw(2) << static_cast<int>(colour.g);
    stream << std::setw(2) << static_cast<int>(colour.b);
    stream << std::setw(2) << static_cast<int>(colour.a);

    return stream.str();
}

/** Parse #rrggbbaa text into an SDL colour */
inline SDL_Color colour(std::string_view colour) {
    if(colour.size() != 9 || colour[0] != '#') {
        throw std::runtime_error(
            "Colour must use #rrggbbaa hex string format"
        );
    }

    const auto channel = [colour](std::size_t index) {
        const std::string value{colour.substr(index, 2)};
        return static_cast<Uint8>(std::stoul(value, nullptr, 16));
    };

    return SDL_Color{
        channel(1),
        channel(3),
        channel(5),
        channel(7),
    };
}

/** Return the last three tokens of a path-like string */
inline std::string path(std::string_view value) {
    std::vector<std::string_view> tokens;
    std::size_t offset = 0;

    while(offset < value.size()) {
        const std::size_t separator = value.find_first_of("/\\", offset);
        const std::size_t end =
            separator == std::string_view::npos ? value.size() : separator;

        if(end > offset) {
            tokens.push_back(value.substr(offset, end - offset));
        }

        if(separator == std::string_view::npos) {
            break;
        }

        offset = separator + 1;
    }

    if(tokens.empty()) {
        return std::string{value};
    }

    const std::size_t first =
        tokens.size() > 3 ? tokens.size() - 3 : 0;
    std::ostringstream stream;

    for(std::size_t index = first; index < tokens.size(); index++) {
        if(index > first) {
            stream << "/";
        }

        stream << tokens[index];
    }

    return stream.str();
}

/** Format a value for compact log output */
template <typename Value>
    requires(!std::is_arithmetic_v<Value>)
inline std::string value(const Value &) {
    return "<value>";
}

inline std::string value(const std::string &value) {
    return "'" + value + "'";
}

inline std::string value(const char *value) {
    return value == nullptr ? "<null>" : "'" + std::string{value} + "'";
}

inline std::string value(bool value) {
    return value ? "true" : "false";
}

template <typename Value>
    requires std::is_arithmetic_v<Value> && (!std::is_same_v<Value, bool>)
inline std::string value(Value value) {
    return std::to_string(value);
}

inline std::string value(const SDL_Point &value) {
    return "[" + std::to_string(value.x) + ", " + std::to_string(value.y)
        + "]";
}

inline std::string value(const SDL_Rect &value) {
    return "[" + std::to_string(value.x) + ", " + std::to_string(value.y)
        + ", " + std::to_string(value.w) + ", " + std::to_string(value.h)
        + "]";
}

inline std::string value(const SDL_Color &value) {
    return colour(value);
}

template <typename Value>
inline std::string value(const std::vector<Value> &value) {
    return "[" + std::to_string(value.size()) + " items]";
}

/** Format a node property value with property-specific rules */
template <typename Value>
inline std::string propertyValue(const std::string &name, const Value &value) {
    return Engine::Format::value(value);
}

inline std::string
propertyValue(const std::string &name, const std::string &value) {
    if(name.find("path") != std::string::npos) {
        return Engine::Format::path(value);
    }

    return Engine::Format::value(value);
}

} // namespace Engine::Format
