#pragma once

#include "../base.hpp"

#include <SDL_ttf.h>

#include <chrono>
#include <memory>
#include <string>

namespace Engine::Resource {

/** Loaded SDL_ttf font at a specific point size */
class Font : public Engine::Resource::Base {
  private:
    struct FontDeleter {
        void operator()(TTF_Font *font) const {
            TTF_CloseFont(font);
        }
    };

    static std::unique_ptr<TTF_Font, FontDeleter>
    load(const std::string &path, int size);

  public:
    Font(const std::string &path, int size);

    /** Time a live cached font can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{120};

    /** Create a cache-owned font resource */
    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path, int size) {
        return std::make_unique<Font>(path, size);
    }

    /** Return the stable cache ID for a font path and size */
    static Engine::Resource::ID key(const std::string &path, int size);

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Owned SDL_ttf font handle */
    const std::unique_ptr<TTF_Font, FontDeleter> handle;

    /** Source font path */
    const std::string path;

    /** Requested font size */
    const int size = 0;
};

} // namespace Engine::Resource
