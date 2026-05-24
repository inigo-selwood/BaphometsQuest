#pragma once

#include "../../resources/types/font.hpp"
#include "../../resources/types/text_texture.hpp"
#include "../../runtime/game.hpp"
#include "../../runtime/render/canvas.hpp"
#include "object.hpp"

#include <SDL.h>

#include <stdexcept>
#include <string>

namespace Engine::Nodes {

/** Render text into the node tree */
class Label : public Engine::Nodes::Object {
  public:
    /** Horizontal anchor used when drawing text at the node position */
    enum class Justification {
        Left,
        Centre,
        Right,
    };

    Label() {
        this->declareHook(Engine::Nodes::Hook::Render);
        this->declareProperty(
            "font",
            this->font,
            [this](const std::string &value) {
                this->update(value, this->fontSize, this->colour, this->text);
            }
        );
        this->declareProperty(
            "font-size",
            this->fontSize,
            [this](const int &value) {
                this->update(this->font, value, this->colour, this->text);
            }
        );
        this->declareProperty(
            "text",
            this->text,
            [this](const std::string &value) {
                this->update(this->font, this->fontSize, this->colour, value);
            }
        );
        this->declareProperty(
            "colour",
            this->colour,
            [this](const SDL_Color &value) {
                this->update(this->font, this->fontSize, value, this->text);
            }
        );
        this->declareProperty("justification", this->justification);
    }

    /** Draw the rendered text texture using the configured justification */
    void render(Engine::Render::Canvas &canvas) override {
        Engine::Game &game = this->getGame();

        if(this->textResourceID == 0) {
            return;
        }

        const Engine::Resource::TextTexture &texture =
            game.resources.get<Engine::Resource::TextTexture>(
                this->textResourceID
            );
        SDL_Rect destination{
            0,
            0,
            texture.size.w,
            texture.size.h,
        };

        switch(this->justification) {
        case Justification::Left:
            destination.x = 0;
            break;
        case Justification::Centre:
            destination.x = -(texture.size.w / 2);
            break;
        case Justification::Right:
            destination.x = -texture.size.w;
            break;
        }

        canvas.copy(texture.handle.get(), nullptr, destination);
    }

  private:
    /** Rebuild cached font and text texture IDs after display input changes */
    void update(
        const std::string &font,
        int size,
        SDL_Color colour,
        const std::string &text
    ) {
        Engine::Game &game = this->getGame();
        const bool fontChanged = font != this->font || size != this->fontSize;
        const bool textureChanged = fontChanged || text != this->text
            || colour.r != this->colour.r || colour.g != this->colour.g
            || colour.b != this->colour.b || colour.a != this->colour.a;

        this->font = font;
        this->fontSize = size;
        this->colour = colour;
        this->text = text;

        if(!textureChanged && this->textResourceID != 0) {
            return;
        }

        this->textResourceID = 0;

        if(this->font.empty() || this->fontSize <= 0) {
            this->fontResourceID = 0;
            return;
        }

        if(this->text.empty()) {
            return;
        }

        if(game.renderer == nullptr) {
            throw std::runtime_error(
                "Label requires a renderer before rebuilding text texture"
            );
        }

        if(fontChanged || this->fontResourceID == 0) {
            this->fontResourceID = game.resources.load<Engine::Resource::Font>(
                this->font,
                this->fontSize
            );
        }

        this->textResourceID =
            game.resources.load<Engine::Resource::TextTexture>(
                game.renderer.get(),
                this->fontResourceID,
                this->colour,
                this->text
            );
    }

    SDL_Color colour{255, 255, 255, 255};
    std::string font;
    int fontSize = 0;
    Justification justification = Justification::Left;
    std::string text;

    Engine::Resource::ID fontResourceID = 0;
    Engine::Resource::ID textResourceID = 0;
};

} // namespace Engine::Nodes
