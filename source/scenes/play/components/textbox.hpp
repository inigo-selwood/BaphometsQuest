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

    void process(float deltaSeconds) override;

    void render(Engine::Render::Canvas &canvas) override;

  private:
    struct Line {
        std::string text;
        Engine::Resource::ID textResourceID = 0;
    };

    void rebuild();

    void rebuildCursor();

    SDL_Rect getCursorDestination() const;

    int getTextWidth() const;

    std::vector<std::string>
    wrapText(const Engine::Resource::Font &font, int width) const;

    static int
    measureText(const Engine::Resource::Font &font, const std::string &text);

    static std::vector<std::string> wrapParagraph(
        const Engine::Resource::Font &font,
        const std::string &paragraph,
        int width
    );

    std::string text;
    bool awaitingInput = false;
    SDL_Color colour{32, 40, 61, 255};
    SDL_Rect size{0, 0, 144, 40};
    std::string font = "resources/fonts/04B_03.TTF";
    int fontSize = 8;
    std::string cursorPath = "resources/textures/tileset.png";
    SDL_Rect cursorRegion{8, 8, 8, 8};
    Engine::Resource::ID fontResourceID = 0;
    Engine::Resource::ID cursorResourceID = 0;
    std::vector<Line> lines;
    float caretElapsed = 0.0F;
    int caretShift = 0;
};

} // namespace Scenes::Play::Components
