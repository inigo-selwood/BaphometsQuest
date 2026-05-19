#pragma once

#include "format.hpp"

#include "../nodes/native/label.hpp"

#include <SDL.h>

#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Engine::Parse {

namespace Detail {

inline std::string_view trim(std::string_view text) {
    const std::size_t start = text.find_first_not_of(" \t\n\r");

    if(start == std::string_view::npos) {
        return {};
    }

    const std::size_t end = text.find_last_not_of(" \t\n\r");

    return text.substr(start, end - start + 1);
}

inline std::string_view arrayBody(
    std::string_view text,
    const std::string &typeName
) {
    text = trim(text);

    if(text.empty()) {
        throw std::runtime_error(typeName + " value must not be empty");
    }

    if(text.front() != '[' && text.back() != ']') {
        return text;
    }

    if(text.front() != '[' || text.back() != ']') {
        throw std::runtime_error(
            typeName + " value has mismatched array brackets: '"
            + std::string(text) + "'"
        );
    }

    return text.substr(1, text.size() - 2);
}

inline int integer(std::string_view text, const std::string &typeName) {
    text = trim(text);

    int value = 0;
    const auto result =
        std::from_chars(text.data(), text.data() + text.size(), value);

    if(result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        throw std::runtime_error(
            typeName + " value must be an integer: '" + std::string(text) + "'"
        );
    }

    return value;
}

inline std::array<int, 4> integerList(
    std::string_view text,
    std::size_t count,
    const std::string &typeName
) {
    text = arrayBody(text, typeName);

    std::array<int, 4> values{0, 0, 0, 0};
    std::size_t offset = 0;

    for(std::size_t index = 0; index < count; index++) {
        const std::size_t separator = text.find(',', offset);
        const bool isLast = index == count - 1;

        if(isLast && separator != std::string_view::npos) {
            throw std::runtime_error(
                typeName + " value has too many components: '"
                + std::string(text) + "'"
            );
        }

        if(!isLast && separator == std::string_view::npos) {
            throw std::runtime_error(
                typeName + " value has too few components: '"
                + std::string(text) + "'"
            );
        }

        const std::size_t end = isLast ? text.size() : separator;
        values[index] = integer(text.substr(offset, end - offset), typeName);
        offset = end + 1;
    }

    return values;
}

} // namespace Detail

/** Parse true/false text */
inline bool boolean(std::string_view text) {
    if(text == "true") {
        return true;
    }

    if(text == "false") {
        return false;
    }

    throw std::runtime_error(
        "Bool value must be true or false: '" + std::string(text) + "'"
    );
}

/** Parse integer text */
inline int integer(std::string_view text) {
    return Detail::integer(text, "Integer");
}

/** Parse x,y text into an SDL point */
inline SDL_Point point(std::string_view text) {
    const auto values = Detail::integerList(text, 2, "Point");

    return SDL_Point{values[0], values[1]};
}

/** Parse x,y,w,h text into an SDL rectangle */
inline SDL_Rect rect(std::string_view text) {
    const auto values = Detail::integerList(text, 4, "Rect");

    return SDL_Rect{values[0], values[1], values[2], values[3]};
}

/** Parse #rrggbbaa text into an SDL colour */
inline SDL_Color colour(std::string_view text) {
    return Engine::Format::colour(text);
}

/** Parse label justification text */
inline Engine::Nodes::Label::Justification
justification(std::string_view text) {
    if(text == "left") {
        return Engine::Nodes::Label::Justification::Left;
    }

    if(text == "centre") {
        return Engine::Nodes::Label::Justification::Centre;
    }

    if(text == "right") {
        return Engine::Nodes::Label::Justification::Right;
    }

    throw std::runtime_error(
        "Justification value must be left, centre, or right: '"
        + std::string(text) + "'"
    );
}

} // namespace Engine::Parse
