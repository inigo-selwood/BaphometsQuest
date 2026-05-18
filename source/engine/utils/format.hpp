#pragma once

#include <SDL.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Engine::Format {

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

} // namespace Engine::Format
