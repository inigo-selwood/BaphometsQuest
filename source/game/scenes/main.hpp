#pragma once

#include "../../engine/nodes/native/colour_box.hpp"
#include "../../engine/nodes/base.hpp"

#include <memory>

namespace Game::Scenes {

/** Default scene used to boot the game */
class Main : public Engine::Nodes::Base {
  public:
    Main() {
        auto box = std::make_shared<Engine::Nodes::Native::ColourBox>();
        box->setProperty("colour", SDL_Color{255, 0, 0, 255});

        this->addChild("red-box", box);
    }
};

} // namespace Game::Scenes
