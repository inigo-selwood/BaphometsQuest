#pragma once

#include "../../runtime/render/context.hpp"
#include "../base.hpp"

#include <SDL.h>

namespace Engine::Nodes {

/** Base node for native objects positioned in screen or world space */
class Object : public Engine::Nodes::Base {
  public:
    Object() {
        this->declareProperty("position", this->position);
    }

    /** Add local position to the inherited render context */
    void applyRenderContext(Engine::Render::Context &context) const override {
        context.addOrigin(this->position);
    }

  protected:
    SDL_Point position{0, 0};
};

} // namespace Engine::Nodes
