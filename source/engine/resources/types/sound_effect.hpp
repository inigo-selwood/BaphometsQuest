#pragma once

#include "../base.hpp"

#include <SDL_mixer.h>

#include <chrono>
#include <memory>
#include <string>

namespace Engine::Resource {

/** Loaded short sound effect resource */
class SoundEffect : public Engine::Resource::Base {
  private:
    struct SoundEffectDeleter {
        void operator()(Mix_Chunk *soundEffect) const {
            Mix_FreeChunk(soundEffect);
        }
    };

    static std::unique_ptr<Mix_Chunk, SoundEffectDeleter>
    load(const std::string &path);

  public:
    explicit SoundEffect(const std::string &path);

    /** Time a live cached sound effect can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{60};

    /** Create a cache-owned sound effect resource */
    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<SoundEffect>(path);
    }

    /** Return the stable cache ID for a sound effect path */
    static Engine::Resource::ID key(const std::string &path);

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Owned SDL_mixer chunk handle */
    const std::unique_ptr<Mix_Chunk, SoundEffectDeleter> handle;

    /** Source sound effect path */
    const std::string path;
};

} // namespace Engine::Resource
