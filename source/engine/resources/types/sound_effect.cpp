#include "sound_effect.hpp"

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<Mix_Chunk, SoundEffect::SoundEffectDeleter>
SoundEffect::load(const std::string &path) {
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
    : handle(load(path)), path(path) {}

Engine::Resource::Key SoundEffect::key(const std::string &path) {
    return hashKey("SoundEffect:" + path);
}

std::string SoundEffect::describe() const {
    ::YAML::Node name;
    name["type"] = "SoundEffect";
    name["path"] = this->path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
