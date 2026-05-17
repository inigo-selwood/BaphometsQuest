#pragma once

#include <SDL.h>

#include <iomanip>
#include <sstream>
#include <string>

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

} // namespace Engine::Format
