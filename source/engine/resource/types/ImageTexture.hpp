#pragma once

#include "../Base.hpp"

#include <SDL.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class ImageTexture : public Engine::Resource::Base {
  public:
    ImageTexture(SDL_Renderer *renderer, const std::string &path);

    SDL_Texture *getHandle() const;
    std::string describe() const override;
    SDL_Rect getSize() const;

  private:
    struct TextureDeleter {
        void operator()(SDL_Texture *texture) const;
    };

    std::unique_ptr<SDL_Texture, TextureDeleter> texture;
    std::string path;
    SDL_Rect size{0, 0, 0, 0};
};

} // namespace Engine::Resource
