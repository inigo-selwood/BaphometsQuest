#pragma once

#include "components/player.hpp"
#include "components/textbox.hpp"
#include "components/world.hpp"

#include "../../engine/nodes/base.hpp"
#include "../../engine/runtime/scene_loader.hpp"

namespace Scenes::Play {

/** Play scene used for in-game map rendering */
class Scene : public Engine::Nodes::Base {
  public:
    void setup() override {
        Engine::SceneLoader loader{*this};
        loader.registerNode<Scenes::Play::Components::Player>("player");
        loader.registerNode<Scenes::Play::Components::Textbox>("textbox");
        loader.registerNode<Scenes::Play::Components::World>("world");
        loader.load("source/scenes/play/play.xml");
    }
};

} // namespace Scenes::Play
