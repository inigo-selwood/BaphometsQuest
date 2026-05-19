#pragma once

#include "../../../engine/nodes/native/image.hpp"

#include <SDL.h>

#include <cstddef>
#include <utility>
#include <vector>

namespace Scenes::MainMenu::Components {

/** Image cursor that tracks the selected main menu option */
class Cursor : public Engine::Nodes::Image {
  public:
    explicit Cursor(std::vector<SDL_Point> options)
        : options(std::move(options)) {
        this->declareHook(Engine::Nodes::Hook::Input);
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
        default:
            break;
        }
    }

    /** Align the cursor with its current option */
    void align() {
        if(this->options.empty()) {
            return;
        }

        this->setProperty("position", this->options[this->selectedOption]);
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
        this->selectedOption = (this->selectedOption + 1) % this->options.size();
        this->align();
    }

    std::vector<SDL_Point> options;
    std::size_t selectedOption = 0;
};

} // namespace Scenes::MainMenu::Components
