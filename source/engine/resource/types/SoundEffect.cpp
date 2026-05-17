#include "SoundEffect.hpp"

#include <stdexcept>

namespace Engine::Resource {

SoundEffect::SoundEffect(const std::string &path)
    : soundEffect(Mix_LoadWAV(path.c_str())), path(path) {
    if(!this->soundEffect) {
        throw std::runtime_error(
            "Failed to load sound effect '" + path + "': " + Mix_GetError()
        );
    }
}

Mix_Chunk *SoundEffect::getHandle() const {
    return this->soundEffect.get();
}

std::string SoundEffect::describe() const {
    ::YAML::Node name;
    name["type"] = "SoundEffect";
    name["path"] = this->path;

    return this->formatDescription(name);
}

void SoundEffect::SoundEffectDeleter::operator()(
    Mix_Chunk *soundEffect
) const {
    Mix_FreeChunk(soundEffect);
}

} // namespace Engine::Resource
