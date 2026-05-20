#pragma once

#include "object.hpp"

#include <SDL.h>

namespace Engine::Nodes {

/** Simple filled rectangle node for render testing */
class Box : public Engine::Nodes::Object {
  public:
    Box() {
        this->declareHook(Engine::Nodes::Hook::Render);
        this->declareProperty("colour", this->colour);
        this->declareProperty("size", this->size);
    }

    void render(SDL_Renderer &renderer) override {
        SDL_SetRenderDrawBlendMode(&renderer, SDL_BLENDMODE_BLEND);
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
    SDL_Color colour{255, 255, 255, 255};
    SDL_Rect size{0, 0, 0, 0};
};

} // namespace Engine::Nodes
