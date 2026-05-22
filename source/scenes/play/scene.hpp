#pragma once

#include "components/player.hpp"
#include "components/textbox.hpp"
#include "components/world.hpp"

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/box.hpp"
#include "../../engine/nodes/native/camera.hpp"
#include "../../engine/nodes/native/canvas_layer.hpp"
#include "../../engine/nodes/native/image.hpp"
#include "../../engine/nodes/native/label.hpp"
#include "../../engine/nodes/native/map.hpp"
#include "../../engine/runtime/scene_loader.hpp"

namespace Scenes::Play {

/** Play scene used for in-game map rendering */
class Scene : public Engine::Nodes::Base {
  public:
    void setup() override {
        Engine::SceneLoader loader{*this};
        loader.registerNode<Engine::Nodes::Box>("box");
        loader.registerNode<Engine::Nodes::Camera>("camera");
        loader.registerNode<Engine::Nodes::CanvasLayer>("canvas-layer");
        loader.registerNode<Engine::Nodes::Image>("image");
        loader.registerNode<Engine::Nodes::Label>("label");
        loader.registerNode<Engine::Nodes::Map>("map");
        loader.registerNode<Scenes::Play::Components::Player>("player");
        loader.registerNode<Scenes::Play::Components::Textbox>("textbox");
        loader.registerNode<Scenes::Play::Components::World>("world");
        loader.load("source/scenes/play/play.xml");
    }
};

} // namespace Scenes::Play
