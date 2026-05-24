#pragma once

#include "components/pause_menu.hpp"
#include "components/player.hpp"
#include "components/select_mode_icon.hpp"
#include "components/status_bar.hpp"
#include "components/textbox.hpp"
#include "components/world.hpp"

#include "../../engine/nodes/base.hpp"
#include "../../engine/runtime/scene_loader.hpp"

#include <SDL.h>

#include <algorithm>
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
        loader.registerNode<Scenes::Play::Components::PauseMenu>(
            "pause-menu-controller"
        );
        loader.registerNode<Scenes::Play::Components::Player>("player");
        loader.registerNode<Scenes::Play::Components::SelectModeIcon>(
            "select-mode-icon"
        );
        loader.registerNode<Scenes::Play::Components::StatusBar>(
            "status-bar"
        );
        loader.registerNode<Scenes::Play::Components::Textbox>("textbox");
        loader.registerNode<Scenes::Play::Components::World>("world");
        loader.load("source/scenes/play/play.xml");

        this->updateSelectionMode();
    }

    void input(const SDL_Event &event) override {
        if(event.type != SDL_KEYDOWN || event.key.repeat != 0) {
            return;
        }

        if(this->isSelectionKey(event)) {
            if(this->selectionActive) {
                this->setSelectionActive(false);
            } else if(!this->getPauseMenu()->isOpen()) {
                this->setSelectionActive(true);
            }
            return;
        }

        if(event.key.keysym.sym == SDLK_TAB) {
            this->cycleSelectMode();
            return;
        }

        if(this->selectionActive) {
            if(event.key.keysym.sym == SDLK_RETURN
                || event.key.keysym.sym == SDLK_KP_ENTER) {
                this->confirmSelection();
                return;
            }

            const SDL_Point movement = this->getSelectionMovement(event);

            if(movement.x != 0 || movement.y != 0) {
                this->moveSelectionCursor(movement);
            }

            return;
        }

        if(event.key.keysym.sym != SDLK_ESCAPE) {
            return;
        }

        const auto pauseMenu = this->getPauseMenu();

        if(pauseMenu->isOpen()) {
            pauseMenu->back();
        } else {
            this->setSelectionActive(false);
            pauseMenu->setOpen(true);
        }
    }

  private:
    enum class SelectMode {
        Select,
        Sword,
        Axe,
        Bow,
        Magic,
    };

    void setTextboxText(const std::string &text) {
        this->getChild("screen-layer.textbox")->setProperty("text", text);
    }

    bool isSelectionKey(const SDL_Event &event) const {
        return event.key.keysym.sym == SDLK_LSHIFT
            || event.key.keysym.sym == SDLK_RSHIFT;
    }

    SDL_Point getSelectionMovement(const SDL_Event &event) const {
        const int step = this->getChild("world-layer.world.player")
                             ->getProperty<int>("step");

        switch(event.key.keysym.sym) {
        case SDLK_UP:
            return SDL_Point{0, -step};
        case SDLK_DOWN:
            return SDL_Point{0, step};
        case SDLK_LEFT:
            return SDL_Point{-step, 0};
        case SDLK_RIGHT:
            return SDL_Point{step, 0};
        default:
            return SDL_Point{0, 0};
        }
    }

    void moveSelectionCursor(SDL_Point movement) {
        const auto cursor = this->getChild("world-layer.world.selection-cursor");
        const SDL_Point position = cursor->getProperty<SDL_Point>("position");

        cursor->setProperty(
            "position",
            this->clampSelectionPosition(SDL_Point{
                position.x + movement.x,
                position.y + movement.y,
            })
        );
    }

    void setSelectionActive(bool active) {
        const auto player = this->getChild("world-layer.world.player");
        const auto cursor = this->getChild("world-layer.world.selection-cursor");

        if(active) {
            cursor->setProperty(
                "position",
                player->getProperty<SDL_Point>("position")
            );
        }

        this->updateSelectionMode();
        cursor->setProperty("active", active);
        player->setProperty("movement-active", !active);
        this->selectionActive = active;
    }

    void confirmSelection() {
        this->setTextboxText("Nothing here.");
        this->setSelectionActive(false);
    }

    void updateSelectionMode() {
        const std::string mode = this->getSelectModeName();

        this->getChild<Scenes::Play::Components::SelectModeIcon>(
                "world-layer.world.selection-cursor"
        )
            ->setMode(mode);
    }

    std::string getSelectModeName() const {
        switch(this->selectMode) {
        case SelectMode::Sword:
            return "sword";
        case SelectMode::Axe:
            return "axe";
        case SelectMode::Bow:
            return "bow";
        case SelectMode::Magic:
            return "magic";
        case SelectMode::Select:
        default:
            return "select";
        }
    }

    void cycleSelectMode() {
        switch(this->selectMode) {
        case SelectMode::Select:
            this->selectMode = SelectMode::Sword;
            break;
        case SelectMode::Sword:
            this->selectMode = SelectMode::Axe;
            break;
        case SelectMode::Axe:
            this->selectMode = SelectMode::Bow;
            break;
        case SelectMode::Bow:
            this->selectMode = SelectMode::Magic;
            break;
        case SelectMode::Magic:
            this->selectMode = SelectMode::Select;
            break;
        }

        if(this->selectionActive) {
            const auto cursor = this->getChild(
                "world-layer.world.selection-cursor"
            );
            cursor->setProperty(
                "position",
                this->clampSelectionPosition(
                    cursor->getProperty<SDL_Point>("position")
                )
            );
        }

        this->updateSelectionMode();
    }

    bool isCloseRangeSelectMode() const {
        return this->selectMode == SelectMode::Sword
            || this->selectMode == SelectMode::Axe;
    }

    SDL_Point clampSelectionPosition(SDL_Point position) const {
        const auto player = this->getChild("world-layer.world.player");
        const SDL_Point playerPosition =
            player->getProperty<SDL_Point>("position");
        const int step = player->getProperty<int>("step");

        if(this->isCloseRangeSelectMode()) {
            return SDL_Point{
                std::clamp(position.x, playerPosition.x - step,
                    playerPosition.x + step),
                std::clamp(position.y, playerPosition.y - step,
                    playerPosition.y + step),
            };
        }

        const SDL_Rect screen = this->getGame().getScreenSize();
        const int halfWidth = screen.w / 2;
        const int halfHeight = screen.h / 2;

        return SDL_Point{
            std::clamp(position.x, playerPosition.x - halfWidth,
                playerPosition.x + halfWidth - step),
            std::clamp(position.y, playerPosition.y - halfHeight,
                playerPosition.y + halfHeight - step),
        };
    }

    std::shared_ptr<Scenes::Play::Components::PauseMenu> getPauseMenu() const {
        return this->getChild<Scenes::Play::Components::PauseMenu>(
            "screen-layer.pause-menu-controller"
        );
    }

    SelectMode selectMode = SelectMode::Select;
    bool selectionActive = false;
};

} // namespace Scenes::Play
