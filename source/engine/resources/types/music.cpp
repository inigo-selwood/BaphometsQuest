#include "music.hpp"

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<Mix_Music, Music::MusicDeleter>
Music::load(const std::string &path) {
    std::unique_ptr<Mix_Music, MusicDeleter> music(Mix_LoadMUS(path.c_str()));

    if(!music) {
        throw std::runtime_error(
            "Failed to load music '" + path + "': " + Mix_GetError()
        );
    }

    return music;
}

Music::Music(const std::string &path) : handle(load(path)), path(path) {}

Engine::Resource::Key Music::key(const std::string &path) {
    return hashKey("Music:" + path);
}

std::string Music::describe() const {
    ::YAML::Node name;
    name["type"] = "Music";
    name["path"] = this->path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
