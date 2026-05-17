#include "Font.hpp"

#include <stdexcept>

namespace Engine::Resource {

Font::Font(const std::string &path, int size)
    : font(TTF_OpenFont(path.c_str(), size)), path(path), size(size) {
    if(!this->font) {
        throw std::runtime_error(
            "Failed to load font '" + path + "': " + TTF_GetError()
        );
    }
}

TTF_Font *Font::getHandle() const {
    return this->font.get();
}

std::string Font::describe() const {
    ::YAML::Node name;
    name["type"] = "Font";
    name["path"] = this->path;
    name["size"] = this->size;

    return this->formatDescription(name);
}

void Font::FontDeleter::operator()(TTF_Font *font) const {
    TTF_CloseFont(font);
}

} // namespace Engine::Resource
