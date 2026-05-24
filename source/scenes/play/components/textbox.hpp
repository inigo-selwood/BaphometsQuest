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

    /** Anchor the textbox and build its initial cached resources */
    void setup() override;

    /** Advance the waiting cursor animation */
    void process(float deltaSeconds) override;

    /** Draw the textbox background, wrapped text, and waiting cursor */
    void render(Engine::Render::Canvas &canvas) override;

  private:
    /** Cached wrapped line of rendered dialogue text */
    struct Line {
        std::string text;
        Engine::Resource::ID textResourceID = 0;
    };

    /** Return the cursor rectangle relative to the textbox */
    SDL_Rect getCursorDestination() const;

    /** Return the text width available beside the cursor */
    int getTextWidth() const;

    /** Rebuild wrapped text resources after display input changes */
    void rebuild();

    /** Rebuild the cached cursor image resource */
    void rebuildCursor();

    /** Anchor the textbox to the bottom of the current screen */
    void updateAnchor();

    /** Split current textbox text into rendered line strings */
    std::vector<std::string>
    wrapText(const Engine::Resource::Font &font, int width) const;

    /** Measure text width for wrapping decisions */
    static int
    measureText(const Engine::Resource::Font &font, const std::string &text);

    /** Split one paragraph into lines that fit the target width */
    static std::vector<std::string> wrapParagraph(
        const Engine::Resource::Font &font,
        const std::string &paragraph,
        int width
    );

    bool awaitingInput = false;
    SDL_Color colour{32, 40, 61, 255};
    std::string font = "resources/fonts/04B_03.TTF";
    int fontSize = 8;
    SDL_Rect size{0, 0, 144, 40};
    std::string text;

    std::string cursorPath = "resources/textures/tileset.png";
    SDL_Rect cursorRegion{8, 8, 8, 8};
    Engine::Resource::ID cursorResourceID = 0;
    float caretElapsed = 0.0F;
    int caretShift = 0;

    Engine::Resource::ID fontResourceID = 0;
    std::vector<Line> lines;
};

} // namespace Scenes::Play::Components
