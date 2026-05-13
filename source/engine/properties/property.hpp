#pragma once

#include <SDL.h>

#include <cstdint>
#include <string>

namespace Engine {

namespace Property {

/** Parse a complete base-10 integer property value. */
int parseInteger(const std::string &value, const std::string &propertyName);

/** Parse #rrggbb or #rrggbbaa into an SDL colour. */
SDL_Color parseColour(const std::string &value, const std::string &name);

/** Parse one two-character hexadecimal byte from a larger string. */
std::uint8_t parseHexByte(
    const std::string &value,
    std::size_t offset,
    const std::string &name
);

/** Parse "x,y" into an SDL point. */
SDL_Point parsePoint(const std::string &value, const std::string &name);

/** Parse "x,y,w,h" into an SDL rectangle. */
SDL_Rect parseRect(const std::string &value, const std::string &name);

} // namespace Property

} // namespace Engine
