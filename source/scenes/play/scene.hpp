#pragma once

#include "components/pause_menu.hpp"

#include "../../engine/nodes/base.hpp"

#include <SDL.h>

#include <memory>
#include <string>

namespace Scenes::Play {

/** Play scene used for in-game map rendering */
class Scene : public Engine::Nodes::Base {
  public:
    Scene();

    /** Load play scene XML and initialize selection UI */
    void setup() override;

    /** Route keyboard input between menus, selection mode, and dialogue */
    void input(const SDL_Event &event) override;

  private:
    /** Current selection cursor tool mode */
    enum class SelectMode {
        Select,
        Sword,
        Axe,
        Bow,
        Magic,
    };

    /** Confirm the current selection cursor target */
    void confirmSelection();

    /** Advance to the next selection cursor tool mode */
    void cycleSelectMode();

    /** Show play-scene dialogue text */
    void showTextbox(const std::string &text);

    /** Hide play-scene dialogue text */
    void hideTextbox();

    /** Clamp selection cursor movement to the active mode bounds */
    SDL_Point clampSelectionPosition(SDL_Point position) const;

    /** Return the XML icon name for the active selection mode */
    std::string getSelectModeName() const;

    /** Return grid movement requested by a keyboard event */
    SDL_Point getSelectionMovement(const SDL_Event &event) const;

    /** Return true when the active mode is limited to adjacent tiles */
    bool isCloseRangeSelectMode() const;

    /** Return true when an event toggles selection mode */
    bool isSelectionKey(const SDL_Event &event) const;

    /** Move the selection cursor by a grid delta */
    void moveSelectionCursor(SDL_Point movement);

    /** Activate or deactivate selection mode */
    void setSelectionActive(bool active);

    /** Update the cursor icon for the active selection mode */
    void updateSelectionMode();

    /** Return the pause menu controller */
    std::shared_ptr<Scenes::Play::Components::PauseMenu> getPauseMenu() const;

    SelectMode selectMode = SelectMode::Select;
    bool selectionActive = false;
};

} // namespace Scenes::Play
