#include "Music.hpp"

#include <stdexcept>

namespace Engine::Resource {

Music::Music(const std::string &path)
    : music(Mix_LoadMUS(path.c_str())), path(path) {
    if(!this->music) {
        throw std::runtime_error(
            "Failed to load music '" + path + "': " + Mix_GetError()
        );
    }
}

Mix_Music *Music::getHandle() const {
    return this->music.get();
}

std::string Music::describe() const {
    ::YAML::Node name;
    name["type"] = "Music";
    name["path"] = this->path;

    return this->formatDescription(name);
}

void Music::MusicDeleter::operator()(Mix_Music *music) const {
    Mix_FreeMusic(music);
}

} // namespace Engine::Resource
