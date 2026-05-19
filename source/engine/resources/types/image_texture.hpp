#pragma once

#include "../base.hpp"

#include <SDL.h>

#include <chrono>
#include <memory>
#include <string>

namespace Engine::Resource {

/** Loaded SDL texture created from an image file */
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

    /** Time a live cached image texture can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{10};

    /** Create a cache-owned image texture resource */
    static std::unique_ptr<Engine::Resource::Base> create(
        Engine::Resource::Manager &,
        SDL_Renderer *renderer,
        const std::string &path
    ) {
        return std::make_unique<ImageTexture>(renderer, path);
    }

    /** Return the stable cache ID for an image path */
    static Engine::Resource::ID
    key(SDL_Renderer *renderer, const std::string &path);

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Owned SDL texture handle */
    const std::unique_ptr<SDL_Texture, TextureDeleter> handle;

    /** Source image path */
    const std::string path;

    /** Texture size as an SDL rectangle with origin zero */
    const SDL_Rect size{0, 0, 0, 0};
};

} // namespace Engine::Resource
