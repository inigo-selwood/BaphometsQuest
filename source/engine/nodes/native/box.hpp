#pragma once

#include "../base.hpp"

#include <SDL.h>

namespace Engine::Nodes {

/** Simple filled rectangle node for render testing */
class Box : public Engine::Nodes::Base {
  public:
    Box() {
        this->declareHook(Engine::Nodes::Hook::Render);
        this->declareProperty("colour", this->colour);
        this->declareProperty("position", this->position);
        this->declareProperty("size", this->size);
    }

    void render(SDL_Renderer &renderer) override {
        SDL_SetRenderDrawColor(
            &renderer,
            this->colour.r,
            this->colour.g,
            this->colour.b,
            this->colour.a
        );
        const SDL_Rect drawArea{this->position.x,
            this->position.y,
            this->size.w,
            this->size.h};
        SDL_RenderFillRect(&renderer, &drawArea);
    }

  private:
    SDL_Point position{0, 0};
    SDL_Color colour{255, 255, 255, 255};
    SDL_Rect size{0, 0, 0, 0};
};

} // namespace Engine::Nodes
