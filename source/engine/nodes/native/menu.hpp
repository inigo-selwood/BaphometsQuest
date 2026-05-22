#pragma once

#include "../../resources/base.hpp"
#include "object.hpp"

#include <SDL.h>

#include <cstddef>
#include <string>
#include <vector>

namespace tinyxml2 {
class XMLElement;
}

namespace Engine::Nodes {

/** Menu node that owns its options, cursor, input, and rendering */
class Menu : public Engine::Nodes::Object {
  public:
    /** Selectable menu option */
    struct Option {
        std::string tag;
        std::string text;
        Engine::Resource::ID textResourceID = 0;
    };

    Menu();

    bool loadXmlChildren(const tinyxml2::XMLElement &element) override;

    /** Remove a selectable option by tag */
    void removeOption(const std::string &tag);

    void input(const SDL_Event &event) override;

    void process(float deltaSeconds) override;

    void render(Engine::Render::Canvas &canvas) override;

  private:
    void rebuild();

    void selectCurrent();

    static Option parseOption(const tinyxml2::XMLElement &optionElement);

    std::string font;
    int fontSize = 0;
    SDL_Color colour{255, 255, 255, 255};
    int lineHeight = 0;
    int cursorOffset = 0;
    std::string cursorPath;
    SDL_Rect cursorRegion{0, 0, 0, 0};
    Engine::Resource::ID fontResourceID = 0;
    Engine::Resource::ID cursorResourceID = 0;
    std::vector<Option> options;
    std::size_t selectedOption = 0;
    float cursorElapsed = 0.0F;
    int cursorShift = 0;
};

} // namespace Engine::Nodes
