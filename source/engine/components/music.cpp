#include "music.hpp"

#include "../format/format.hpp"
#include "../game/game.hpp"

#include <SDL_mixer.h>

#include <algorithm>
#include <stdexcept>

namespace Engine {

Music::Music() {
    this->registerProperty<bool>(
        "auto-play",
        "Music",
        [this] { return this->autoPlay; },
        [this](const bool &value) { this->autoPlay = value; }
    );
    this->registerProperty<int>(
        "fade-in-ms",
        "Music",
        [this] { return this->fadeInMS; },
        [this](const int &value) { this->fadeInMS = std::max(0, value); }
    );
    this->registerProperty<int>(
        "loops",
        "Music",
        [this] { return this->loops; },
        [this](const int &value) { this->loops = value; }
    );
    this->registerProperty<std::string>(
        "path",
        "Music",
        [this] { return this->path; },
        [this](const std::string &value) {
            this->path = value;
            this->musicID.reset();
        },
        [](const std::string &value) { return Format::filePath(value); }
    );
    this->registerProperty<bool>(
        "stop-on-exit",
        "Music",
        [this] { return this->stopOnExit; },
        [this](const bool &value) { this->stopOnExit = value; }
    );
    this->registerProperty<int>(
        "volume",
        "Music",
        [this] { return this->volume; },
        [this](const int &value) { this->setVolume(value); }
    );
}

void Music::registerType() {
    Node::registerType<Music>("Music");
}

void Music::play() {
    this->ensureLoaded();

    auto &assets = Game::getInstance().getAssets();
    auto &music = assets.get<Mix_Music>(*this->musicID);

    Mix_VolumeMusic(this->volume);

    const int result = this->fadeInMS > 0
        ? Mix_FadeInMusic(&music, this->loops, this->fadeInMS)
        : Mix_PlayMusic(&music, this->loops);

    if(result != 0) {
        throw std::runtime_error(
            std::string("Failed to play music '") + this->path
            + "': " + Mix_GetError()
        );
    }
}

void Music::stop() {
    if(Mix_PlayingMusic() == 0) {
        return;
    }

    Mix_HaltMusic();
}

void Music::ensureLoaded() {
    if(this->musicID.has_value()) {
        return;
    }

    if(this->path.empty()) {
        throw std::runtime_error("Music node is missing required path.");
    }

    this->musicID = Game::getInstance().getAssets().loadMusic(this->path);
}

void Music::onEnterTree() {
    if(this->autoPlay) {
        this->play();
    }
}

void Music::onExitTree() {
    if(this->stopOnExit) {
        this->stop();
    }
}

void Music::setVolume(int value) {
    if(value < 0 || value > MIX_MAX_VOLUME) {
        throw std::runtime_error("Music volume must be between 0 and 128.");
    }

    this->volume = value;
}

} // namespace Engine
