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

    void setup() override;
    void input(const SDL_Event &event) override;

  private:
    enum class SelectMode {
        Select,
        Sword,
        Axe,
        Bow,
        Magic,
    };

    void confirmSelection();
    void cycleSelectMode();

    void showTextbox(const std::string &text);
    void hideTextbox();

    SDL_Point clampSelectionPosition(SDL_Point position) const;
    std::string getSelectModeName() const;
    SDL_Point getSelectionMovement(const SDL_Event &event) const;
    bool isCloseRangeSelectMode() const;
    bool isSelectionKey(const SDL_Event &event) const;

    void moveSelectionCursor(SDL_Point movement);
    void setSelectionActive(bool active);
    void updateSelectionMode();

    std::shared_ptr<Scenes::Play::Components::PauseMenu> getPauseMenu() const;

    SelectMode selectMode = SelectMode::Select;
    bool selectionActive = false;
};

} // namespace Scenes::Play
