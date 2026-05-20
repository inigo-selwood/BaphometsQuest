#pragma once

#include "../../../engine/nodes/native/image.hpp"

#include <SDL.h>

namespace Scenes::Play::Components {

/** Temporary controllable player image for camera traversal */
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

        switch(event.key.keysym.sym) {
        case SDLK_UP:
            this->position.y -= this->step;
            break;
        case SDLK_DOWN:
            this->position.y += this->step;
            break;
        case SDLK_LEFT:
            this->position.x -= this->step;
            break;
        case SDLK_RIGHT:
            this->position.x += this->step;
            break;
        default:
            break;
        }
    }

  private:
    int step = 8;
};

} // namespace Scenes::Play::Components
