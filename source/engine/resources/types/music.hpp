#pragma once

#include "../base.hpp"

#include <SDL_mixer.h>

#include <chrono>
#include <memory>
#include <string>

namespace Engine::Resource {

/** Loaded streaming music resource */
class Music : public Engine::Resource::Base {
  private:
    struct MusicDeleter {
        void operator()(Mix_Music *music) const {
            Mix_FreeMusic(music);
        }
    };

    static std::unique_ptr<Mix_Music, MusicDeleter>
    load(const std::string &path);

  public:
    explicit Music(const std::string &path);

    /** Negative TTL keeps music live until clear() */
    static constexpr std::chrono::seconds TTL{-1};

    /** Create a cache-owned music resource */
    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<Music>(path);
    }

    /** Return the stable cache ID for a music path */
    static Engine::Resource::ID key(const std::string &path);

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Owned SDL_mixer music handle */
    const std::unique_ptr<Mix_Music, MusicDeleter> handle;

    /** Source music path */
    const std::string path;
};

} // namespace Engine::Resource
