#pragma once

#include "../base.hpp"
#include "font.hpp"

#include <SDL.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class TextTexture : public Engine::Resource::Base {
  private:
    struct TextureDeleter {
        void operator()(SDL_Texture *texture) const {
            SDL_DestroyTexture(texture);
        }
    };

    static std::unique_ptr<SDL_Texture, TextureDeleter> render(
        SDL_Renderer *renderer,
        const Engine::Resource::Font &font,
        SDL_Color colour,
        const std::string &text
    );

    static SDL_Rect querySize(SDL_Texture *texture, const std::string &text);

  public:
    TextTexture(
        SDL_Renderer *renderer,
        const Engine::Resource::Font &font,
        SDL_Color colour,
        const std::string &text
    );

    std::string describe() const override;

    const std::unique_ptr<SDL_Texture, TextureDeleter> handle;
    const std::string fontDescription;
    const SDL_Color colour{0, 0, 0, 0};
    const SDL_Rect size{0, 0, 0, 0};
    const std::string text;
};

} // namespace Engine::Resource
