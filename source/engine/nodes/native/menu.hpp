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

    /** Load menu options from XML child elements */
    bool loadXmlChildren(const tinyxml2::XMLElement &element) override;

    /** Remove a selectable option by tag */
    void removeOption(const std::string &tag);

    /** Handle keyboard navigation and selection */
    void input(const SDL_Event &event) override;

    /** Advance cursor animation state */
    void process(float deltaSeconds) override;

    /** Draw menu labels and cursor */
    void render(Engine::Render::Canvas &canvas) override;

  private:
    /** Parse one XML option child */
    static Option parseOption(const tinyxml2::XMLElement &optionElement);

    /** Rebuild cached text and cursor resources */
    void rebuild();

    /** Emit the selected option signal */
    void selectCurrent();

    SDL_Color colour{255, 255, 255, 255};
    std::string font;
    int fontSize = 0;

    int lineHeight = 0;

    int cursorOffset = 0;
    std::string cursorPath;
    SDL_Rect cursorRegion{0, 0, 0, 0};
    Engine::Resource::ID cursorResourceID = 0;
    float cursorElapsed = 0.0F;
    int cursorShift = 0;

    Engine::Resource::ID fontResourceID = 0;

    std::vector<Option> options;
    std::size_t selectedOption = 0;
};

} // namespace Engine::Nodes
