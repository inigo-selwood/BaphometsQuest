#include "music.hpp"

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<Mix_Music, Music::MusicDeleter>
Music::loadMusic(const std::string &path) {
    std::unique_ptr<Mix_Music, MusicDeleter> music(Mix_LoadMUS(path.c_str()));

    if(!music) {
        throw std::runtime_error(
            "Failed to load music '" + path + "': " + Mix_GetError()
        );
    }

    return music;
}

Music::Music(const std::string &path) : Handle(loadMusic(path)), Path(path) {}

std::string Music::describe() const {
    ::YAML::Node name;
    name["type"] = "Music";
    name["path"] = this->Path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
