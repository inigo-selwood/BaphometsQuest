#pragma once

#include "../Base.hpp"

#include <SDL_mixer.h>

#include <memory>
#include <string>

namespace Engine::Resource {

class Music : public Engine::Resource::Base {
  public:
    explicit Music(const std::string &path);

    Mix_Music *getHandle() const;
    std::string describe() const override;

  private:
    struct MusicDeleter {
        void operator()(Mix_Music *music) const;
    };

    std::unique_ptr<Mix_Music, MusicDeleter> music;
    std::string path;
};

} // namespace Engine::Resource
