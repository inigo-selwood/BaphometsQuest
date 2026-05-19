#pragma once

#include "../base.hpp"
#include "font.hpp"

#include <SDL.h>

#include <chrono>
#include <memory>
#include <string>

namespace Engine::Resource {

/** SDL texture rendered from text and a cached font resource */
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
        Engine::Resource::ID fontID,
        const Engine::Resource::Font &font,
        SDL_Color colour,
        const std::string &text
    );

    /** Time a live cached text texture can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{10};

    /** Create a cache-owned text texture resource */
    static std::unique_ptr<Engine::Resource::Base> create(
        Engine::Resource::Manager &manager,
        SDL_Renderer *renderer,
        Engine::Resource::ID fontID,
        SDL_Color colour,
        const std::string &text
    );

    /** Return the stable cache ID for a rendered text texture */
    static Engine::Resource::ID
    key(SDL_Renderer *renderer,
        Engine::Resource::ID fontID,
        SDL_Color colour,
        const std::string &text);

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Owned SDL texture handle */
    const std::unique_ptr<SDL_Texture, TextureDeleter> handle;

    /** Cached font resource ID used to reconstruct this texture */
    const Engine::Resource::ID fontID;

    /** Text colour used when rendering the texture */
    const SDL_Color colour{0, 0, 0, 0};

    /** Texture size as an SDL rectangle with origin zero */
    const SDL_Rect size{0, 0, 0, 0};

    /** Text content rendered into the texture */
    const std::string text;
};

} // namespace Engine::Resource
