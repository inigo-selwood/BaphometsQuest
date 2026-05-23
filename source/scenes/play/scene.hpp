#pragma once

#include "components/player.hpp"
#include "components/textbox.hpp"
#include "components/world.hpp"

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/menu.hpp"
#include "../../engine/runtime/scene_loader.hpp"

#include <SDL.h>

#include <memory>
#include <string>

namespace Scenes::Play {

/** Play scene used for in-game map rendering */
class Scene : public Engine::Nodes::Base {
  public:
    Scene() {
        this->declareHook(Engine::Nodes::Hook::Input);
    }

    void setup() override {
        Engine::SceneLoader loader{*this};
        loader.registerNode<Scenes::Play::Components::Player>("player");
        loader.registerNode<Scenes::Play::Components::Textbox>("textbox");
        loader.registerNode<Scenes::Play::Components::World>("world");
        loader.load("source/scenes/play/play.xml");

        const auto screenLayer = this->getChild("screen-layer");
        const auto pauseOverlay = screenLayer->getChild("pause-overlay");
        const auto pauseMenu = this->getChild<Engine::Nodes::Menu>(
            "screen-layer.pause-overlay.pause-menu"
        );
        const auto saveConfirmation =
            screenLayer->getChild("save-confirmation");
        const auto saveMenu = this->getChild<Engine::Nodes::Menu>(
            "screen-layer.save-confirmation.save-menu"
        );

        this->getGame().signals.connect<std::string>(
            pauseMenu,
            "selected",
            [this, pauseOverlay, saveConfirmation](const std::string &tag) {
                if(tag == "inventory") {
                    this->setTextboxText("Inventory is empty.");
                } else if(tag == "stats") {
                    this->setTextboxText("Stats are unavailable.");
                } else if(tag == "save") {
                    pauseOverlay->setProperty("active", false);
                    saveConfirmation->setProperty("active", true);
                } else if(tag == "main-menu") {
                    this->getGame().queueScene("main");
                }
            }
        );

        this->getGame().signals.connect<std::string>(
            saveMenu,
            "selected",
            [this, pauseOverlay, saveConfirmation](const std::string &tag) {
                if(tag == "yes") {
                    this->persistPlayerState();
                    this->getGame().saveState();
                    this->setTextboxText("Game saved.");
                    pauseOverlay->setProperty("active", true);
                    saveConfirmation->setProperty("active", false);
                } else if(tag == "back") {
                    pauseOverlay->setProperty("active", true);
                    saveConfirmation->setProperty("active", false);
                }
            }
        );
    }

    void input(const SDL_Event &event) override {
        if(event.type != SDL_KEYDOWN || event.key.repeat != 0) {
            return;
        }

        if(event.key.keysym.sym != SDLK_ESCAPE) {
            return;
        }

        const auto pauseOverlay = this->getChild("screen-layer.pause-overlay");
        const auto saveConfirmation =
            this->getChild("screen-layer.save-confirmation");

        if(saveConfirmation->isActive()) {
            pauseOverlay->setProperty("active", true);
            saveConfirmation->setProperty("active", false);
            return;
        }

        this->setPaused(!pauseOverlay->isActive());
    }

  private:
    void setPaused(bool paused) {
        this->getChild("world-layer")->setProperty("active", !paused);
        this->getChild("screen-layer.pause-overlay")
            ->setProperty("active", paused);
        this->getChild("screen-layer.save-confirmation")
            ->setProperty("active", false);
        this->getChild("screen-layer.textbox")->setProperty("active", !paused);
    }

    void setTextboxText(const std::string &text) {
        this->getChild("screen-layer.textbox")->setProperty("text", text);
    }

    void persistPlayerState() {
        const SDL_Point playerPosition =
            this->getChild("world-layer.world.player")
                ->getProperty<SDL_Point>("position");

        this->getGame().state.set("player-position", playerPosition);
    }
};

} // namespace Scenes::Play
