#pragma once

#include "../../../engine/nodes/native/image.hpp"
#include "../../../engine/runtime/game.hpp"

#include <SDL.h>

#include <cstddef>
#include <string>
#include <vector>

namespace Scenes::MainMenu::Components {

/** Image cursor that tracks the selected main menu option */
class Cursor : public Engine::Nodes::Image {
  public:
    /** Named position that the cursor can select */
    struct Option {
        std::string tag;
        SDL_Point position;
    };

    Cursor() {
        this->declareHook(Engine::Nodes::Hook::Enter);
        this->declareHook(Engine::Nodes::Hook::Input);
        this->declareProperty(
            "options",
            this->options,
            [this](const std::vector<Option> &value) {
                this->updateOptions(value);
            }
        );
    }

    void enter() override {
        this->getGame().signals.declare<std::string>(
            this->shared_from_this(),
            "selected"
        );
    }

    void input(const SDL_Event &event) override {
        if(event.type != SDL_KEYDOWN || this->options.empty()) {
            return;
        }

        switch(event.key.keysym.sym) {
        case SDLK_UP:
            this->selectPrevious();
            break;
        case SDLK_DOWN:
            this->selectNext();
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            this->selectCurrent();
            break;
        default:
            break;
        }
    }

    /** Align the cursor with its current option */
    void align() {
        if(this->options.empty()) {
            return;
        }

        this->setProperty(
            "position",
            this->options[this->selectedOption].position
        );
    }

  private:
    void selectPrevious() {
        if(this->selectedOption == 0) {
            this->selectedOption = this->options.size() - 1;
        } else {
            this->selectedOption--;
        }

        this->align();
    }

    void selectNext() {
        this->selectedOption =
            (this->selectedOption + 1) % this->options.size();
        this->align();
    }

    void selectCurrent() {
        this->getGame().signals.emit(
            this->shared_from_this(),
            "selected",
            this->options[this->selectedOption].tag
        );
    }

    void updateOptions(const std::vector<Option> &value) {
        this->options = value;
        this->selectedOption = 0;
        this->align();
    }

    std::vector<Option> options;
    std::size_t selectedOption = 0;
};

} // namespace Scenes::MainMenu::Components
