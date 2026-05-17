#include "sound_effect.hpp"

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<Mix_Chunk, SoundEffect::SoundEffectDeleter>
SoundEffect::loadSoundEffect(const std::string &path) {
    std::unique_ptr<Mix_Chunk, SoundEffectDeleter> soundEffect(
        Mix_LoadWAV(path.c_str())
    );

    if(!soundEffect) {
        throw std::runtime_error(
            "Failed to load sound effect '" + path + "': " + Mix_GetError()
        );
    }

    return soundEffect;
}

SoundEffect::SoundEffect(const std::string &path)
    : Handle(loadSoundEffect(path)), Path(path) {}

std::string SoundEffect::describe() const {
    ::YAML::Node name;
    name["type"] = "SoundEffect";
    name["path"] = this->Path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
