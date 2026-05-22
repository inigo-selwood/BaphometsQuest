#pragma once

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/label.hpp"
#include "../../engine/nodes/native/menu.hpp"
#include "../../engine/nodes/native/music.hpp"
#include "../../engine/nodes/native/sprite.hpp"
#include "../../engine/runtime/scene_loader.hpp"

#include <string>

namespace Scenes::MainMenu {

/** Main menu scene used to boot the game */
class Scene : public Engine::Nodes::Base {
  public:
    void setup() override {
        Engine::SceneLoader loader{*this};
        loader.registerNode<Engine::Nodes::Label>("label");
        loader.registerNode<Engine::Nodes::Menu>("menu");
        loader.registerNode<Engine::Nodes::Music>("music");
        loader.registerNode<Engine::Nodes::Sprite>("sprite");
        loader.load("source/scenes/main_menu/main_menu.xml");

        const auto menu = this->getChild<Engine::Nodes::Menu>("main-menu");

        if(!this->getGame().state.has("player-position")) {
            menu->removeOption("continue");
        }

        this->getGame().signals.connect<std::string>(
            menu,
            "selected",
            [this](const std::string &tag) {
                if(tag == "continue") {
                    this->getGame().queueScene("play");
                } else if(tag == "new-game") {
                    this->getGame().queueScene("new-game");
                } else if(tag == "quit") {
                    this->getGame().queueQuit();
                }
            }
        );
    }
};

} // namespace Scenes::MainMenu
