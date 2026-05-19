#pragma once

#include "../../runtime/game.hpp"
#include "../base.hpp"
#include "menu_cursor.hpp"
#include "menu_option.hpp"

#include <SDL.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Engine::Nodes {

/** Container that wires menu options to a cursor and selected signal */
class Menu : public Engine::Nodes::Base {
  public:
    Menu() {
        this->declareProperty("font", this->font);
        this->declareProperty("size", this->size);
        this->declareProperty("colour", this->colour);
        this->declareProperty("position", this->position);
    }

    /** Configure cursor options and declare the selected signal */
    void configure() {
        if(this->configured) {
            return;
        }

        Engine::Game &game = this->getGame();
        const std::shared_ptr<Base> owner = this->shared_from_this();
        const std::weak_ptr<Base> ownerReference = owner;
        std::shared_ptr<MenuCursor> cursor;
        std::vector<MenuCursor::Option> options;
        int optionIndex = 0;

        for(const auto &child : this->getChildren()) {
            if(const auto childCursor =
                   std::dynamic_pointer_cast<MenuCursor>(child)) {
                if(cursor != nullptr) {
                    throw std::runtime_error(
                        "Menu must not contain multiple cursors"
                    );
                }

                cursor = childCursor;
                continue;
            }

            if(const auto option =
                   std::dynamic_pointer_cast<MenuOption>(child)) {
                const int verticalOffset = optionIndex * this->getRowHeight();
                const SDL_Point optionPosition{
                    this->position.x,
                    this->position.y + verticalOffset,
                };
                const SDL_Point cursorPosition{
                    this->position.x - this->getCursorOffset(),
                    this->position.y + verticalOffset,
                };

                option->setProperty("font", this->font);
                option->setProperty("size", this->size);
                option->setProperty("colour", this->colour);
                option->setProperty("position", optionPosition);
                options.push_back(
                    MenuCursor::Option{
                        option->getTag(),
                        cursorPosition,
                    }
                );
                optionIndex++;
            }
        }

        if(cursor == nullptr) {
            throw std::runtime_error("Menu requires a cursor child");
        }

        if(options.empty()) {
            throw std::runtime_error("Menu requires at least one option child");
        }

        game.signals.declare<std::string>(owner, "selected");
        cursor->configure();
        cursor->setProperty("options", options);
        game.signals.connect<std::string>(
            cursor,
            "selected",
            [&game, ownerReference](std::string tag) {
                const std::shared_ptr<Base> owner = ownerReference.lock();

                if(owner == nullptr) {
                    return;
                }

                game.signals.emit(owner, "selected", tag);
            }
        );

        this->configured = true;
    }

  private:
    int getRowHeight() const {
        return this->size * 2;
    }

    int getCursorOffset() const {
        return this->size * 2;
    }

    std::string font;
    int size = 0;
    SDL_Color colour{255, 255, 255, 255};
    SDL_Point position{0, 0};
    bool configured = false;
};

} // namespace Engine::Nodes
