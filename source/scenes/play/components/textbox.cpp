#include "textbox.hpp"

#include "../../../engine/resources/types/font.hpp"
#include "../../../engine/resources/types/image_texture.hpp"
#include "../../../engine/resources/types/text_texture.hpp"
#include "../../../engine/runtime/game.hpp"
#include "../../../engine/runtime/render/canvas.hpp"

#include <SDL_ttf.h>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Scenes::Play::Components {

namespace {

constexpr SDL_Color TEXT_COLOUR{255, 255, 255, 255};
constexpr SDL_Point TEXT_ORIGIN{2, 2};
constexpr int LINE_HEIGHT = 8;
constexpr int MAX_LINES = 4;
constexpr int TEXT_CURSOR_GAP = 2;
constexpr float CARET_OSCILLATION_INTERVAL = 0.5F;

} // namespace

Textbox::Textbox() {
    this->declareHook(Engine::Nodes::Hook::Process);
    this->declareHook(Engine::Nodes::Hook::Render);
    this->declareProperty(
        "text",
        this->text,
        [this](const std::string &value) {
            this->text = value;
            this->rebuild();
        }
    );
    this->declareProperty("awaiting-input", this->awaitingInput);
    this->declareProperty("colour", this->colour);
    this->declareProperty(
        "size",
        this->size,
        [this](const SDL_Rect &value) {
            this->size = value;
            this->rebuild();
        }
    );
    this->declareProperty(
        "font",
        this->font,
        [this](const std::string &value) {
            this->font = value;
            this->rebuild();
        }
    );
    this->declareProperty("font-size", this->fontSize, [this](int value) {
        this->fontSize = value;
        this->rebuild();
    });
    this->declareProperty(
        "cursor-path",
        this->cursorPath,
        [this](const std::string &value) {
            this->cursorPath = value;
            this->rebuildCursor();
        }
    );
    this->declareProperty("cursor-region", this->cursorRegion);
}

void Textbox::setup() {
    const SDL_Rect screen = this->getGame().getScreenSize();
    this->position = SDL_Point{0, screen.h - this->size.h};
    this->rebuildCursor();
}

void Textbox::process(float deltaSeconds) {
    if(!this->awaitingInput) {
        this->caretElapsed = 0.0F;
        this->caretShift = 0;
        return;
    }

    this->caretElapsed += deltaSeconds;

    while(this->caretElapsed >= CARET_OSCILLATION_INTERVAL) {
        this->caretElapsed -= CARET_OSCILLATION_INTERVAL;
        this->caretShift = this->caretShift == 0 ? 1 : 0;
    }
}

void Textbox::render(Engine::Render::Canvas &canvas) {
    Engine::Game &game = this->getGame();

    canvas.setBlendMode(SDL_BLENDMODE_BLEND);
    canvas.setDrawColour(this->colour);
    canvas.fillRect(this->size);

    for(std::size_t index = 0; index < this->lines.size(); index++) {
        const Line &line = this->lines[index];

        if(line.textResourceID == 0) {
            continue;
        }

        const Engine::Resource::TextTexture &texture =
            game.resources.get<Engine::Resource::TextTexture>(
                line.textResourceID
        );
        const SDL_Rect destination{
            TEXT_ORIGIN.x,
            TEXT_ORIGIN.y + (static_cast<int>(index) * LINE_HEIGHT),
            texture.size.w,
            texture.size.h,
        };

        canvas.copy(texture.handle.get(), nullptr, destination);
    }

    if(!this->awaitingInput || this->cursorResourceID == 0) {
        return;
    }

    const Engine::Resource::ImageTexture &cursor =
        game.resources.get<Engine::Resource::ImageTexture>(
            this->cursorResourceID
        );

    SDL_Rect destination = this->getCursorDestination();
    destination.y += this->caretShift;

    canvas.copy(cursor.handle.get(), &this->cursorRegion, destination);
}

void Textbox::rebuild() {
    this->lines.clear();

    if(this->text.empty()) {
        this->fontResourceID = 0;
        return;
    }

    if(this->font.empty() || this->fontSize <= 0) {
        this->fontResourceID = 0;
        return;
    }

    Engine::Game &game = this->getGame();

    if(game.renderer == nullptr) {
        throw std::runtime_error(
            "Textbox requires a renderer before rebuilding text"
        );
    }

    this->fontResourceID = game.resources.load<Engine::Resource::Font>(
        this->font,
        this->fontSize
    );

    const Engine::Resource::Font &fontResource =
        game.resources.get<Engine::Resource::Font>(this->fontResourceID);
    const std::vector<std::string> wrappedLines =
        this->wrapText(fontResource, this->getTextWidth());
    const std::size_t visibleLines = std::min(
        wrappedLines.size(),
        static_cast<std::size_t>(MAX_LINES)
    );

    for(std::size_t index = 0; index < visibleLines; index++) {
        this->lines.push_back(Line{
            wrappedLines[index],
            game.resources.load<Engine::Resource::TextTexture>(
                game.renderer.get(),
                this->fontResourceID,
                TEXT_COLOUR,
                wrappedLines[index]
            ),
        });
    }
}

void Textbox::rebuildCursor() {
    Engine::Game &game = this->getGame();

    if(this->cursorPath.empty()) {
        this->cursorResourceID = 0;
        return;
    }

    if(game.renderer == nullptr) {
        throw std::runtime_error(
            "Textbox requires a renderer before rebuilding cursor"
        );
    }

    this->cursorResourceID =
        game.resources.load<Engine::Resource::ImageTexture>(
            game.renderer.get(),
            this->cursorPath
        );
}

SDL_Rect Textbox::getCursorDestination() const {
    return SDL_Rect{
        this->size.w - this->cursorRegion.w - 2,
        this->size.h - this->cursorRegion.h - 2,
        this->cursorRegion.w,
        this->cursorRegion.h,
    };
}

int Textbox::getTextWidth() const {
    return std::max(
        0,
        this->size.w - TEXT_ORIGIN.x - this->cursorRegion.w - TEXT_CURSOR_GAP
    );
}

std::vector<std::string>
Textbox::wrapText(const Engine::Resource::Font &font, int width) const {
    std::vector<std::string> wrappedLines;
    std::size_t start = 0;

    while(start <= this->text.size()) {
        const std::size_t end = this->text.find('\n', start);
        const std::string paragraph = this->text.substr(
            start,
            end == std::string::npos ? std::string::npos : end - start
        );
        std::vector<std::string> paragraphLines =
            wrapParagraph(font, paragraph, width);

        wrappedLines.insert(
            wrappedLines.end(),
            paragraphLines.begin(),
            paragraphLines.end()
        );

        if(end == std::string::npos) {
            break;
        }

        start = end + 1;
    }

    return wrappedLines;
}

int Textbox::measureText(
    const Engine::Resource::Font &font,
    const std::string &text
) {
    int width = 0;
    int height = 0;

    if(TTF_SizeUTF8(font.handle.get(), text.c_str(), &width, &height) != 0) {
        throw std::runtime_error(
            "Failed to measure textbox text '" + text + "': " + TTF_GetError()
        );
    }

    return width;
}

std::vector<std::string> Textbox::wrapParagraph(
    const Engine::Resource::Font &font,
    const std::string &paragraph,
    int width
) {
    std::vector<std::string> lines;
    std::istringstream stream{paragraph};
    std::string word;
    std::string currentLine;

    while(stream >> word) {
        const std::string candidate =
            currentLine.empty() ? word : currentLine + " " + word;

        if(!currentLine.empty() && measureText(font, candidate) > width) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = candidate;
        }
    }

    if(!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}

} // namespace Scenes::Play::Components
