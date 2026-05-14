#pragma once

#include <SDL.h>

#include <array>
#include <iomanip>
#include <ostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace Engine {

namespace Format {

template <typename ValueType>
concept Streamable = requires(std::ostream &stream, const ValueType &value) {
    stream << value;
};

template <typename ValueType> std::string value(const ValueType &value) {
    if constexpr(std::is_enum_v<ValueType>) {
        return std::to_string(
            static_cast<std::underlying_type_t<ValueType>>(value)
        );
    } else if constexpr(Streamable<ValueType>) {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    } else {
        return "<unprintable>";
    }
}

inline std::string boolean(bool value) {
    return value ? "true" : "false";
}

template <typename ValueType>
std::string array(std::span<const ValueType> values) {
    std::ostringstream stream;
    stream << "[";

    for(std::size_t index = 0; index < values.size(); index++) {
        if(index > 0) {
            stream << ", ";
        }

        stream << values[index];
    }

    stream << "]";
    return stream.str();
}

template <typename ValueType, std::size_t Size>
std::string array(const std::array<ValueType, Size> &values) {
    return array(std::span<const ValueType>(values));
}

inline std::string colour(const SDL_Color &value) {
    std::ostringstream stream;
    stream << "#";
    stream << std::hex << std::nouppercase << std::setfill('0');
    stream << std::setw(2) << static_cast<int>(value.r);
    stream << std::setw(2) << static_cast<int>(value.g);
    stream << std::setw(2) << static_cast<int>(value.b);
    stream << std::setw(2) << static_cast<int>(value.a);
    return stream.str();
}

inline std::string
filePath(std::string_view path, std::size_t maximumLength = 48) {
    if(path.size() <= maximumLength) {
        return std::string(path);
    }

    if(maximumLength <= 3) {
        return std::string(path.substr(path.size() - maximumLength));
    }

    return "..." + std::string(path.substr(path.size() - (maximumLength - 3)));
}

inline std::string point(const SDL_Point &value) {
    return array(std::array<int, 2>{value.x, value.y});
}

inline std::string rect(const SDL_Rect &value) {
    return array(std::array<int, 4>{value.x, value.y, value.w, value.h});
}

} // namespace Format

} // namespace Engine
