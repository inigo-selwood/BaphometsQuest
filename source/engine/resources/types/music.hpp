#pragma once

#include "../base.hpp"

#include <SDL_mixer.h>

#include <chrono>
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
    load(const std::string &path);

  public:
    explicit Music(const std::string &path);

    static constexpr std::chrono::seconds TTL{60};

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<Music>(path);
    }

    static Engine::Resource::Key key(const std::string &path);

    std::string describe() const override;

    const std::unique_ptr<Mix_Music, MusicDeleter> handle;
    const std::string path;
};

} // namespace Engine::Resource
