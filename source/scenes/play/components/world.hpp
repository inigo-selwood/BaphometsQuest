#pragma once

#include "../../../engine/nodes/native/map.hpp"
#include "../../../engine/nodes/native/object.hpp"

#include <SDL.h>

#include <memory>

namespace Scenes::Play::Components {

/** Coordinate gameplay between play-scene world children */
class World : public Engine::Nodes::Object {
  public:
    /** Return true when movement between world-local pixels is permitted */
    bool canMove(SDL_Point fromPixel, SDL_Point toPixel) const {
        const std::shared_ptr<Engine::Nodes::Map> map =
            this->getChild<Engine::Nodes::Map>("map");
        const SDL_Point mapPosition =
            map->getProperty<SDL_Point>("position");

        return map->canMove(
            SDL_Point{
                fromPixel.x - mapPosition.x,
                fromPixel.y - mapPosition.y,
            },
            SDL_Point{
                toPixel.x - mapPosition.x,
                toPixel.y - mapPosition.y,
            }
        );
    }
};

} // namespace Scenes::Play::Components
