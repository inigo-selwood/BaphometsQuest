#pragma once

#include "../base.hpp"

#include <SDL.h>

#include <chrono>
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
    load(SDL_Renderer *renderer, const std::string &path);

    static SDL_Rect querySize(SDL_Texture *texture, const std::string &path);

  public:
    ImageTexture(SDL_Renderer *renderer, const std::string &path);

    static constexpr std::chrono::seconds TTL{10};

    static std::unique_ptr<Engine::Resource::Base> create(
        Engine::Resource::Manager &,
        SDL_Renderer *renderer,
        const std::string &path
    ) {
        return std::make_unique<ImageTexture>(renderer, path);
    }

    static Engine::Resource::Key
    key(SDL_Renderer *renderer, const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<SDL_Texture, TextureDeleter> handle;
    const std::string path;
    const SDL_Rect size{0, 0, 0, 0};
};

} // namespace Engine::Resource
