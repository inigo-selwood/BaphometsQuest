#pragma once

#include "../Base.hpp"
#include "Font.hpp"

#include <SDL.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class TextTexture : public Engine::Resource::Base {
  public:
    TextTexture(
        SDL_Renderer *renderer,
        const Engine::Resource::Font &font,
        SDL_Color colour,
        const std::string &text
    );

    SDL_Texture *getHandle() const;
    std::string describe() const override;
    SDL_Rect getSize() const;

  private:
    struct TextureDeleter {
        void operator()(SDL_Texture *texture) const;
    };

    std::unique_ptr<SDL_Texture, TextureDeleter> texture;
    std::string fontDescription;
    SDL_Color colour{0, 0, 0, 0};
    SDL_Rect size{0, 0, 0, 0};
    std::string text;
};

} // namespace Engine::Resource
