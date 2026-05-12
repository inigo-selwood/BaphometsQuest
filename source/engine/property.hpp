#pragma once

#include "includes.hpp"

#include <cstdint>
#include <string>

namespace Property {

int parseInteger(const std::string &value, const std::string &propertyName);
SDL_Color parseColour(const std::string &value, const std::string &name);
std::uint8_t parseHexByte(
        const std::string &value, std::size_t offset, const std::string &name);
SDL_Point parsePoint(const std::string &value, const std::string &name);
SDL_Rect parseRect(const std::string &value, const std::string &name);

} // namespace Property
