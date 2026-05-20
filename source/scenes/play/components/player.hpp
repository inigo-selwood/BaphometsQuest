#pragma once

#include "../../../engine/nodes/native/image.hpp"
#include "../../../engine/nodes/native/map.hpp"

#include <SDL.h>

#include <memory>

namespace Scenes::Play::Components {

/** Controllable player image for map traversal */
class Player : public Engine::Nodes::Image {
  public:
    Player() {
        this->declareHook(Engine::Nodes::Hook::Input);
        this->declareProperty("step", this->step);
    }

    void input(const SDL_Event &event) override {
        if(event.type != SDL_KEYDOWN) {
            return;
        }

        SDL_Point movement{0, 0};

        switch(event.key.keysym.sym) {
        case SDLK_UP:
            movement.y = -this->step;
            break;
        case SDLK_DOWN:
            movement.y = this->step;
            break;
        case SDLK_LEFT:
            movement.x = -this->step;
            break;
        case SDLK_RIGHT:
            movement.x = this->step;
            break;
        default:
            return;
        }

        const std::shared_ptr<Engine::Nodes::Map> map =
            this->getTreeNode<Engine::Nodes::Map>();
        const SDL_Point mapPosition = map->getProperty<SDL_Point>("position");
        const SDL_Point target{
            this->position.x + movement.x,
            this->position.y + movement.y,
        };

        if(map->canMove(
               SDL_Point{
                   this->position.x - mapPosition.x,
                   this->position.y - mapPosition.y,
               },
               SDL_Point{
                   target.x - mapPosition.x,
                   target.y - mapPosition.y,
               }
           )) {
            this->position = target;
        }
    }

  private:
    int step = 8;
};

} // namespace Scenes::Play::Components
