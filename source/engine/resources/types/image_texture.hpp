#pragma once

#include "../base.hpp"

#include <SDL.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class ImageTexture : public Engine::Resource::Base {
  private:
    struct TextureDeleter {
        void operator()(SDL_Texture *texture) const {
            SDL_DestroyTexture(texture);
        }
    };

    static std::unique_ptr<SDL_Texture, TextureDeleter>
    loadTexture(SDL_Renderer *renderer, const std::string &path);

    static SDL_Rect
    queryTextureSize(SDL_Texture *texture, const std::string &path);

  public:
    ImageTexture(SDL_Renderer *renderer, const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<SDL_Texture, TextureDeleter> Handle;
    const std::string Path;
    const SDL_Rect Size{0, 0, 0, 0};
};

} // namespace Engine::Resource
