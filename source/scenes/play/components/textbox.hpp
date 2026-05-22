#pragma once

#include "../../../engine/nodes/native/object.hpp"
#include "../../../engine/resources/base.hpp"

#include <SDL.h>

#include <string>
#include <vector>

namespace Engine::Resource {
class Font;
}

namespace Scenes::Play::Components {

/** Play-scene dialogue box anchored to the bottom of the screen */
class Textbox : public Engine::Nodes::Object {
  public:
    Textbox();

    void setup() override;

    void render(Engine::Render::Canvas &canvas) override;

  private:
    struct Line {
        std::string text;
        Engine::Resource::ID textResourceID = 0;
    };

    void rebuild();

    void rebuildCursor();

    std::vector<std::string>
    wrapText(const Engine::Resource::Font &font, int width) const;

    static int measureText(
        const Engine::Resource::Font &font,
        const std::string &text
    );

    static std::vector<std::string>
    wrapParagraph(
        const Engine::Resource::Font &font,
        const std::string &paragraph,
        int width
    );

    std::string text;
    bool awaitingInput = false;
    Engine::Resource::ID fontResourceID = 0;
    Engine::Resource::ID cursorResourceID = 0;
    std::vector<Line> lines;
};

} // namespace Scenes::Play::Components
