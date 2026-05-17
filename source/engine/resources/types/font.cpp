#include "font.hpp"

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<TTF_Font, Font::FontDeleter>
Font::loadFont(const std::string &path, int size) {
    std::unique_ptr<TTF_Font, FontDeleter> font(
        TTF_OpenFont(path.c_str(), size)
    );

    if(!font) {
        throw std::runtime_error(
            "Failed to load font '" + path + "': " + TTF_GetError()
        );
    }

    return font;
}

Font::Font(const std::string &path, int size)
    : handle(loadFont(path, size)), path(path), size(size) {}

std::string Font::describe() const {
    ::YAML::Node name;
    name["type"] = "Font";
    name["path"] = this->path;
    name["size"] = this->size;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
