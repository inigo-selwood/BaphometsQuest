#pragma once

#include "../../resources/types/music.hpp"
#include "../../runtime/game.hpp"
#include "../base.hpp"

#include <SDL_mixer.h>

#include <stdexcept>
#include <string>

namespace Engine::Nodes {

/** Play music from the node tree */
class Music : public Engine::Nodes::Base {
  public:
    Music() {
        this->declareHook(Engine::Nodes::Hook::Enter);
        this->declareHook(Engine::Nodes::Hook::Exit);
        this->declareProperty(
            "path",
            this->path,
            [this](const std::string &value) { this->update(value); }
        );
        this->declareProperty("autoplay", this->autoplay);
        this->declareProperty("loop", this->loop);
    }

    /** Start playback when autoplay is enabled */
    void enter() override {
        if(this->autoplay) {
            this->start();
        }
    }

    /** Stop playback when leaving the active tree */
    void exit() override {
        this->stop();
    }

    /** Start playback using the current music resource */
    void start() {
        if(this->musicResourceID == 0) {
            throw std::runtime_error("Music requires a path before starting");
        }

        Engine::Game &game = this->getGame();

        Engine::Resource::Music &music =
            game.resources.get<Engine::Resource::Music>(this->musicResourceID);

        if(Mix_PlayMusic(music.handle.get(), this->loop ? -1 : 0) != 0) {
            throw std::runtime_error(
                std::string("Failed to play music node: ") + Mix_GetError()
            );
        }

        this->active = true;
    }

    /** Stop playback when this node currently owns active music */
    void stop() {
        if(this->active || Mix_PlayingMusic() != 0) {
            Mix_HaltMusic();
        }

        this->active = false;
    }

    /** Pause playback without clearing the active state */
    void pause() {
        if(!this->active) {
            return;
        }

        Mix_PauseMusic();
    }

  private:
    /** Update the cached music ID when the music path changes */
    void update(const std::string &path) {
        const bool musicChanged = path != this->path;

        this->path = path;

        if(!musicChanged) {
            return;
        }

        this->musicResourceID = 0;

        if(this->path.empty()) {
            return;
        }

        Engine::Game &game = this->getGame();

        this->musicResourceID =
            game.resources.load<Engine::Resource::Music>(this->path);

        if(this->active) {
            this->start();
        }
    }

    bool active = false;
    bool autoplay = false;
    bool loop = false;
    std::string path;

    Engine::Resource::ID musicResourceID = 0;
};

} // namespace Engine::Nodes
