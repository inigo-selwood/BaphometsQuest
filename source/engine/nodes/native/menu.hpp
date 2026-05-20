#pragma once

#include "../../resources/base.hpp"
#include "../base.hpp"

#include <SDL.h>

#include <cstddef>
#include <string>
#include <vector>

namespace tinyxml2 {
class XMLElement;
}

namespace Engine::Nodes {

/** Menu node that owns its options, cursor, input, and rendering */
class Menu : public Engine::Nodes::Base {
  public:
    /** Selectable menu option */
    struct Option {
        std::string tag;
        std::string text;
        Engine::Resource::ID textResourceID = 0;
    };

    Menu();

    bool loadXmlChildren(const tinyxml2::XMLElement &element) override;

    void input(const SDL_Event &event) override;

    void render(SDL_Renderer &renderer) override;

  private:
    void rebuild();

    void selectCurrent();

    static Option parseOption(const tinyxml2::XMLElement &optionElement);

    std::string font;
    int size = 0;
    SDL_Color colour{255, 255, 255, 255};
    SDL_Point position{0, 0};
    int lineHeight = 0;
    int cursorOffset = 0;
    std::string cursorPath;
    SDL_Rect cursorRegion{0, 0, 0, 0};
    Engine::Resource::ID fontResourceID = 0;
    Engine::Resource::ID cursorResourceID = 0;
    std::vector<Option> options;
    std::size_t selectedOption = 0;
};

} // namespace Engine::Nodes
