#pragma once

#include "../../resources/types/font.hpp"
#include "../../resources/types/text_texture.hpp"
#include "../../runtime/game.hpp"
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
                this->update(value, this->size, this->colour, this->text);
            }
        );
        this->declareProperty("size", this->size, [this](const int &value) {
            this->update(this->font, value, this->colour, this->text);
        });
        this->declareProperty(
            "text",
            this->text,
            [this](const std::string &value) {
                this->update(this->font, this->size, this->colour, value);
            }
        );
        this->declareProperty(
            "colour",
            this->colour,
            [this](const SDL_Color &value) {
                this->update(this->font, this->size, value, this->text);
            }
        );
        this->declareProperty("justification", this->justification);
    }

    void render(SDL_Renderer &renderer) override {
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
            this->position.y,
            texture.size.w,
            texture.size.h,
        };

        switch(this->justification) {
        case Justification::Left:
            destination.x = this->position.x;
            break;
        case Justification::Centre:
            destination.x = this->position.x - (texture.size.w / 2);
            break;
        case Justification::Right:
            destination.x = this->position.x - texture.size.w;
            break;
        }

        if(SDL_RenderCopy(
               &renderer,
               texture.handle.get(),
               nullptr,
               &destination
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to render label node: ") + SDL_GetError()
            );
        }
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
        const bool fontChanged = font != this->font || size != this->size;
        const bool textureChanged = fontChanged || text != this->text
            || colour.r != this->colour.r || colour.g != this->colour.g
            || colour.b != this->colour.b || colour.a != this->colour.a;

        this->font = font;
        this->size = size;
        this->colour = colour;
        this->text = text;

        if(!textureChanged && this->textResourceID != 0) {
            return;
        }

        this->textResourceID = 0;

        if(this->font.empty() || this->size <= 0) {
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
                this->size
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

    Engine::Resource::ID fontResourceID = 0;
    Engine::Resource::ID textResourceID = 0;
    std::string font;
    int size = 0;
    std::string text;
    SDL_Color colour{255, 255, 255, 255};
    Justification justification = Justification::Left;
};

} // namespace Engine::Nodes
