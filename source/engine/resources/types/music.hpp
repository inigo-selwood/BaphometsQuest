#pragma once

#include "../base.hpp"

#include <SDL_mixer.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class Music : public Engine::Resource::Base {
  private:
    struct MusicDeleter {
        void operator()(Mix_Music *music) const {
            Mix_FreeMusic(music);
        }
    };

    static std::unique_ptr<Mix_Music, MusicDeleter>
    loadMusic(const std::string &path);

  public:
    explicit Music(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<Mix_Music, MusicDeleter> handle;
    const std::string path;
};

} // namespace Engine::Resource
