#pragma once

#include "../Base.hpp"

#include <SDL_mixer.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class SoundEffect : public Engine::Resource::Base {
  public:
    explicit SoundEffect(const std::string &path);

    Mix_Chunk *getHandle() const;
    std::string describe() const override;

  private:
    struct SoundEffectDeleter {
        void operator()(Mix_Chunk *soundEffect) const;
    };

    std::unique_ptr<Mix_Chunk, SoundEffectDeleter> soundEffect;
    std::string path;
};

} // namespace Engine::Resource
