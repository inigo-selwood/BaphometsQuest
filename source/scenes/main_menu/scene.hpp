#pragma once

#include "components/cursor.hpp"

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/label.hpp"
#include "../../engine/nodes/native/music.hpp"
#include "../../engine/runtime/scene_loader.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace Scenes::MainMenu {

/** Main menu scene used to boot the game */
class Scene : public Engine::Nodes::Base {
  public:
    Scene() {
        this->declareHook(Engine::Nodes::Hook::Enter);
    }

    void enter() override {
        if(this->loaded) {
            return;
        }

        Engine::SceneLoader loader{*this};
        loader.registerNode<Engine::Nodes::Label>("label");
        loader.registerNode<Engine::Nodes::Music>("music");
        loader.registerNode<Components::Cursor>("cursor");
        loader.load("source/scenes/main_menu/main_menu.xml");

        const auto cursor = std::dynamic_pointer_cast<Components::Cursor>(
            this->getChild("cursor")
        );

        if(cursor == nullptr) {
            throw std::runtime_error("Main menu XML created invalid nodes");
        }

        cursor->setProperty(
            "options",
            std::vector<Components::Cursor::Option>{
                {"play", {48, 76}},
                {"quit", {48, 92}},
            }
        );

        this->loaded = true;
    }

  private:
    bool loaded = false;
};

} // namespace Scenes::MainMenu
