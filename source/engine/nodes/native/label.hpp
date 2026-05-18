#pragma once

#include "../../resources/types/font.hpp"
#include "../../resources/types/text_texture.hpp"
#include "../../runtime/game.hpp"
#include "../base.hpp"

#include <SDL.h>

#include <stdexcept>
#include <string>

namespace Engine::Nodes {

/** Render text into the node tree */
class Label : public Engine::Nodes::Base {
  public:
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
        this->declareProperty("position", this->position);
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
        const SDL_Rect destination{
            this->position.x,
            this->position.y,
            texture.size.w,
            texture.size.h,
        };

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
    SDL_Point position{0, 0};
};

} // namespace Engine::Nodes
