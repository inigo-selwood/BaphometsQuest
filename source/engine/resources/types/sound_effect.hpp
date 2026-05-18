#pragma once

#include "../base.hpp"

#include <SDL_mixer.h>

#include <chrono>
#include <memory>
#include <string>

namespace Engine::Resource {

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

    static constexpr std::chrono::seconds TTL{60};

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<SoundEffect>(path);
    }

    static Engine::Resource::Key key(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<Mix_Chunk, SoundEffectDeleter> handle;
    const std::string path;
};

} // namespace Engine::Resource
