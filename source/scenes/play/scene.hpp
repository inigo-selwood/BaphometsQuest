#pragma once

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/tilemap.hpp"
#include "../../engine/runtime/scene_loader.hpp"

namespace Scenes::Play {

/** Play scene used for in-game map rendering */
class Scene : public Engine::Nodes::Base {
  public:
    void setup() override {
        Engine::SceneLoader loader{*this};
        loader.registerNode<Engine::Nodes::Tilemap>("tilemap");
        loader.load("source/scenes/play/play.xml");
    }
};

} // namespace Scenes::Play
