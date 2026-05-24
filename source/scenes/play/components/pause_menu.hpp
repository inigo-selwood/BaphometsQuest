#pragma once

#include "../../../engine/nodes/base.hpp"
#include "../../../engine/nodes/native/menu.hpp"
#include "../../../engine/runtime/game.hpp"

#include <SDL.h>

#include <string>

namespace Scenes::Play::Components {

/** Play-scene pause and save menu controller */
class PauseMenu : public Engine::Nodes::Base {
  public:
    /** Connect pause and save menu signals */
    void setup() override {
        const auto pauseOverlay = this->getChild("pause-overlay");
        const auto pauseMenu =
            this->getChild<Engine::Nodes::Menu>("pause-overlay.pause-menu");
        const auto saveConfirmation = this->getChild("save-confirmation");
        const auto saveMenu =
            this->getChild<Engine::Nodes::Menu>("save-confirmation.save-menu");

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

    /** Return true when any pause-owned overlay is active */
    bool isOpen() const {
        return this->getChild("pause-overlay")->isActive()
            || this->getChild("save-confirmation")->isActive();
    }

    /** Toggle the primary pause overlay */
    void toggle() {
        this->setOpen(!this->isOpen());
    }

    /** Open or close the pause overlay */
    void setOpen(bool open) {
        this->getGame()
            .currentScene->getChild("world-layer")
            ->setProperty("active", !open);
        this->getGame()
            .currentScene->getChild("screen-layer.textbox")
            ->setProperty("active", !open);
        this->getChild("pause-overlay")->setProperty("active", open);
        this->getChild("save-confirmation")->setProperty("active", false);
    }

    /** Return from save confirmation or close the pause menu */
    void back() {
        if(this->getChild("save-confirmation")->isActive()) {
            this->getChild("pause-overlay")->setProperty("active", true);
            this->getChild("save-confirmation")->setProperty("active", false);
            return;
        }

        this->setOpen(false);
    }

  private:
    /** Show feedback text through the scene textbox */
    void setTextboxText(const std::string &text) {
        const auto textbox =
            this->getGame().currentScene->getChild("screen-layer.textbox");

        textbox->setProperty("text", text);
        textbox->setProperty("active", true);
    }

    /** Store the current player position before saving */
    void persistPlayerState() {
        const SDL_Point playerPosition =
            this->getGame()
                .currentScene->getChild("world-layer.world.player")
                ->getProperty<SDL_Point>("position");

        this->getGame().state.set("player-position", playerPosition);
    }
};

} // namespace Scenes::Play::Components
