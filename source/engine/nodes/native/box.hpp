#pragma once

#include "../../runtime/render/canvas.hpp"
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

    void render(Engine::Render::Canvas &canvas) override {
        canvas.setBlendMode(SDL_BLENDMODE_BLEND);
        canvas.setDrawColour(this->colour);
        canvas.fillRect(SDL_Rect{0, 0, this->size.w, this->size.h});
    }

  private:
    SDL_Color colour{255, 255, 255, 255};
    SDL_Rect size{0, 0, 0, 0};
};

} // namespace Engine::Nodes
