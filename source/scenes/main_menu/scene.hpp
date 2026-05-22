#pragma once

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/menu.hpp"
#include "../../engine/runtime/game.hpp"
#include "../../engine/runtime/scene_loader.hpp"

#include <SDL.h>
#include <memory>
#include <string>

namespace Scenes::MainMenu {

/** Main menu scene used to boot the game */
class Scene : public Engine::Nodes::Base {
  public:
    void setup() override {
        Engine::SceneLoader loader{*this};
        loader.load("source/scenes/main_menu/main_menu.xml");

        const auto menu = this->getChild<Engine::Nodes::Menu>("main-menu");
        const auto confirmation = this->getChild("new-game-confirmation");
        const auto confirmationMenu =
            confirmation->getChild<Engine::Nodes::Menu>("new-game-menu");

        if(!this->getGame().state.has("player-position")) {
            menu->removeOption("continue");
        }

        this->getGame().signals.connect<std::string>(
            menu,
            "selected",
            [this, menu, confirmation](const std::string &tag) {
                if(tag == "continue") {
                    this->getGame().queueScene("play");
                } else if(tag == "new-game") {
                    this->queueNewGame(menu, confirmation);
                } else if(tag == "quit") {
                    this->getGame().queueQuit();
                }
            }
        );

        this->getGame().signals.connect<std::string>(
            confirmationMenu,
            "selected",
            [this, menu, confirmation](const std::string &tag) {
                if(tag == "continue") {
                    this->startNewGame();
                } else if(tag == "back") {
                    menu->setProperty("active", true);
                    confirmation->setProperty("active", false);
                }
            }
        );
    }

  private:
    void queueNewGame(
        const std::shared_ptr<Engine::Nodes::Menu> &menu,
        const std::shared_ptr<Engine::Nodes::Base> &confirmation
    ) {
        menu->setProperty("active", false);
        confirmation->setProperty("active", true);
    }

    void startNewGame() {
        Engine::Game &game = this->getGame();

        game.state.clear();
        game.state.ensure("player-position", SDL_Point{80, 72});
        game.saveState();
        game.queueScene("play");
    }
};

} // namespace Scenes::MainMenu
