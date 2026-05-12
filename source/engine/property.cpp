#include "property.hpp"

#include <array>
#include <charconv>
#include <stdexcept>
#include <string_view>

namespace Property {

int parseInteger(const std::string &value, const std::string &propertyName) {
    int result = 0;
    const auto *begin = value.data();
    const auto *end = value.data() + value.size();
    const auto parseResult = std::from_chars(begin, end, result);

    if (parseResult.ec != std::errc() || parseResult.ptr != end) {
        throw std::runtime_error("Invalid integer value for property '"
                + propertyName + "': " + value);
    }

    return result;
}

std::uint8_t parseHexByte(const std::string &value, std::size_t offset,
        const std::string &name) {
    int result = 0;
    const auto *begin = value.data() + offset;
    const auto *end = begin + 2;
    const auto parseResult = std::from_chars(begin, end, result, 16);

    if (parseResult.ec != std::errc() || parseResult.ptr != end) {
        throw std::runtime_error(
                "Invalid colour value for property '" + name + "': " + value);
    }

    return static_cast<std::uint8_t>(result);
}

SDL_Point parsePoint(const std::string &value, const std::string &name) {
    const std::size_t separator = value.find(',');

    if (separator == std::string::npos || separator == 0
            || separator == value.size() - 1) {
        throw std::runtime_error(
                "Invalid point value for property '" + name + "': " + value);
    }

    return SDL_Point{
            parseInteger(value.substr(0, separator), name),
            parseInteger(value.substr(separator + 1), name),
    };
}

SDL_Rect parseRect(const std::string &value, const std::string &name) {
    std::array<int, 4> values{};
    std::size_t start = 0;

    for (std::size_t index = 0; index < values.size(); index++) {
        const std::size_t separator = value.find(',', start);
        const bool lastValue = index == values.size() - 1;

        if (lastValue && separator != std::string::npos) {
            throw std::runtime_error("Invalid rect value for property '" + name
                    + "': " + value);
        }

        if (!lastValue && separator == std::string::npos) {
            throw std::runtime_error("Invalid rect value for property '" + name
                    + "': " + value);
        }

        const std::size_t end = lastValue ? value.size() : separator;

        if (end == start) {
            throw std::runtime_error("Invalid rect value for property '" + name
                    + "': " + value);
        }

        values[index] = parseInteger(value.substr(start, end - start), name);
        start = end + 1;
    }

    return SDL_Rect{values[0], values[1], values[2], values[3]};
}

SDL_Color parseColour(const std::string &value, const std::string &name) {
    if (value.size() != 7 && value.size() != 9) {
        throw std::runtime_error(
                "Invalid colour value for property '" + name + "': " + value);
    }

    if (value[0] != '#') {
        throw std::runtime_error(
                "Invalid colour value for property '" + name + "': " + value);
    }

    return SDL_Color{
            parseHexByte(value, 1, name),
            parseHexByte(value, 3, name),
            parseHexByte(value, 5, name),
            value.size() == 9 ? parseHexByte(value, 7, name)
                              : static_cast<std::uint8_t>(255),
    };
}

} // namespace Property
