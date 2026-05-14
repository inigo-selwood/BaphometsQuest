#pragma once

#include <SDL.h>

#include <cstdint>
#include <string>

namespace Engine {

namespace Parse {

/** Parse #rrggbb or #rrggbbaa into an SDL colour. */
SDL_Color colour(const std::string &value, const std::string &name);

/** Parse true/false into a boolean. */
bool boolean(const std::string &value, const std::string &name);

/** Parse a complete floating-point value. */
float floating(const std::string &value, const std::string &name);

/** Parse a complete base-10 integer value. */
int integer(const std::string &value, const std::string &name);

/** Parse "x,y" into an SDL point. */
SDL_Point point(const std::string &value, const std::string &name);

/** Parse "x,y,w,h" into an SDL rectangle. */
SDL_Rect rect(const std::string &value, const std::string &name);

} // namespace Parse

} // namespace Engine
