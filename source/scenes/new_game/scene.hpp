#pragma once

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/label.hpp"
#include "../../engine/nodes/native/menu.hpp"
#include "../../engine/runtime/game.hpp"
#include "../../engine/runtime/scene_loader.hpp"

#include <SDL.h>
#include <string>

namespace Scenes::NewGame {

/** Transition scene that creates a fresh save before entering play */
class Scene : public Engine::Nodes::Base {
  public:
    void setup() override {
        if(!this->getGame().state.has("player-position")) {
            this->startNewGame();
            return;
        }

        Engine::SceneLoader loader{*this};
        loader.registerNode<Engine::Nodes::Label>("label");
        loader.registerNode<Engine::Nodes::Menu>("menu");
        loader.load("source/scenes/new_game/new_game.xml");

        const auto menu =
            this->getChild<Engine::Nodes::Menu>("new-game-menu");

        this->getGame().signals.connect<std::string>(
            menu,
            "selected",
            [this](const std::string &tag) {
                if(tag == "continue") {
                    this->startNewGame();
                } else if(tag == "back") {
                    this->getGame().queueScene("main");
                }
            }
        );
    }

  private:
    void startNewGame() {
        Engine::Game &game = this->getGame();

        game.state.clear();
        game.state.ensure("player-position", SDL_Point{80, 72});
        game.saveState();
        game.queueScene("play");
    }
};

} // namespace Scenes::NewGame
