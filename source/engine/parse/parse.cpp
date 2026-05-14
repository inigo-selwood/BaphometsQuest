#include "parse.hpp"

#include <array>
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <stdexcept>

namespace Engine {

namespace Parse {

namespace {

std::uint8_t hexByte(
    const std::string &value,
    std::size_t offset,
    const std::string &name
) {
    int result = 0;
    const auto *begin = value.data() + offset;
    const auto *end = begin + 2;
    const auto parseResult = std::from_chars(begin, end, result, 16);

    if(parseResult.ec != std::errc() || parseResult.ptr != end) {
        throw std::runtime_error(
            "Invalid colour value for '" + name + "': " + value
        );
    }

    return static_cast<std::uint8_t>(result);
}

} // namespace

SDL_Color colour(const std::string &value, const std::string &name) {
    if(value.size() != 7 && value.size() != 9) {
        throw std::runtime_error(
            "Invalid colour value for '" + name + "': " + value
        );
    }

    if(value[0] != '#') {
        throw std::runtime_error(
            "Invalid colour value for '" + name + "': " + value
        );
    }

    return SDL_Color{
        hexByte(value, 1, name),
        hexByte(value, 3, name),
        hexByte(value, 5, name),
        value.size() == 9 ? hexByte(value, 7, name)
                          : static_cast<std::uint8_t>(255),
    };
}

bool boolean(const std::string &value, const std::string &name) {
    if(value == "true") {
        return true;
    }

    if(value == "false") {
        return false;
    }

    throw std::runtime_error(
        "Invalid boolean value for '" + name + "': " + value
    );
}

float floating(const std::string &value, const std::string &name) {
    char *end = nullptr;
    errno = 0;
    const float result = std::strtof(value.c_str(), &end);

    if(errno != 0 || end == value.c_str() || *end != '\0') {
        throw std::runtime_error(
            "Invalid floating-point value for '" + name + "': " + value
        );
    }

    return result;
}

int integer(const std::string &value, const std::string &name) {
    int result = 0;
    const auto *begin = value.data();
    const auto *end = value.data() + value.size();
    const auto parseResult = std::from_chars(begin, end, result);

    if(parseResult.ec != std::errc() || parseResult.ptr != end) {
        throw std::runtime_error(
            "Invalid integer value for '" + name + "': " + value
        );
    }

    return result;
}

SDL_Point point(const std::string &value, const std::string &name) {
    const std::size_t separator = value.find(',');

    if(separator == std::string::npos || separator == 0
        || separator == value.size() - 1) {
        throw std::runtime_error(
            "Invalid point value for '" + name + "': " + value
        );
    }

    return SDL_Point{
        integer(value.substr(0, separator), name),
        integer(value.substr(separator + 1), name),
    };
}

SDL_Rect rect(const std::string &value, const std::string &name) {
    std::array<int, 4> values{};
    std::size_t start = 0;

    for(std::size_t index = 0; index < values.size(); index++) {
        const std::size_t separator = value.find(',', start);
        const bool lastValue = index == values.size() - 1;

        if(lastValue && separator != std::string::npos) {
            throw std::runtime_error(
                "Invalid rect value for '" + name + "': " + value
            );
        }

        if(!lastValue && separator == std::string::npos) {
            throw std::runtime_error(
                "Invalid rect value for '" + name + "': " + value
            );
        }

        const std::size_t end = lastValue ? value.size() : separator;

        if(end == start) {
            throw std::runtime_error(
                "Invalid rect value for '" + name + "': " + value
            );
        }

        values[index] = integer(value.substr(start, end - start), name);
        start = end + 1;
    }

    return SDL_Rect{values[0], values[1], values[2], values[3]};
}

} // namespace Parse

} // namespace Engine
