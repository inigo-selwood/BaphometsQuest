#pragma once

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/label.hpp"
#include "../../engine/nodes/native/menu.hpp"
#include "../../engine/nodes/native/menu_cursor.hpp"
#include "../../engine/nodes/native/menu_option.hpp"
#include "../../engine/nodes/native/music.hpp"
#include "../../engine/runtime/scene_loader.hpp"

#include <memory>
#include <string>

namespace Scenes::MainMenu {

/** Main menu scene used to boot the game */
class Scene : public Engine::Nodes::Base {
  public:
    void setup() override {
        Engine::SceneLoader loader{*this};
        loader.registerNode<Engine::Nodes::Label>("label");
        loader.registerNode<Engine::Nodes::Menu>("menu");
        loader.registerNode<Engine::Nodes::MenuCursor>("menu-cursor");
        loader.registerNode<Engine::Nodes::MenuOption>("menu-option");
        loader.registerNode<Engine::Nodes::Music>("music");
        loader.load("source/scenes/main_menu/main_menu.xml");

        const auto menu = this->getChild<Engine::Nodes::Menu>("main-menu");

        menu->configure();
        this->getGame().signals.connect<std::string>(
            menu,
            "selected",
            [this](const std::string &tag) {
                if(tag == "quit") {
                    this->getGame().queueQuit();
                }
            }
        );
    }
};

} // namespace Scenes::MainMenu
