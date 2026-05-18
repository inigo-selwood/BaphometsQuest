#pragma once

#include "../base.hpp"

#include <SDL.h>

namespace Engine::Nodes::Native {

/** Simple filled rectangle node for render testing */
class ColourBox : public Engine::Nodes::Base {
  public:
    ColourBox() {
        this->declareHook(Engine::Nodes::Hook::Render);
        this->declareProperty("colour", this->colour);
        this->declareProperty("rectangle", this->rectangle);
    }

    void render(SDL_Renderer &renderer) override {
        SDL_SetRenderDrawColor(
            &renderer,
            this->colour.r,
            this->colour.g,
            this->colour.b,
            this->colour.a
        );
        SDL_RenderFillRect(&renderer, &this->rectangle);
    }

    SDL_Color colour{255, 255, 255, 255};
    SDL_Rect rectangle{32, 32, 128, 128};
};

} // namespace Engine::Nodes::Native
