#pragma once

#include "../../runtime/render/context.hpp"
#include "../base.hpp"

#include <stdexcept>
#include <string>

namespace Engine::Nodes {

/** Node that defines the coordinate mode for its subtree */
class CanvasLayer : public Engine::Nodes::Base {
  public:
    CanvasLayer() {
        this->declareProperty(
            "mode",
            this->mode,
            [this](const std::string &value) { this->setMode(value); }
        );
    }

    /** Reset inherited origin and apply this layer's canvas mode */
    void applyRenderContext(Engine::Render::Context &context) const override {
        context.resetOrigin();
        context.mode = this->canvasMode;
    }

  private:
    /** Parse the XML canvas mode into render context state */
    void setMode(const std::string &mode) {
        if(mode == "screen") {
            this->mode = mode;
            this->canvasMode = Engine::Render::CanvasMode::Screen;
            return;
        }

        if(mode == "world") {
            this->mode = mode;
            this->canvasMode = Engine::Render::CanvasMode::World;
            return;
        }

        throw std::runtime_error(
            "Canvas layer mode must be 'screen' or 'world'"
        );
    }

    std::string mode = "screen";
    Engine::Render::CanvasMode canvasMode = Engine::Render::CanvasMode::Screen;
};

} // namespace Engine::Nodes
