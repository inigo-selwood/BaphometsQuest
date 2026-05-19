#include "font.hpp"

#include "../../utils/format.hpp"

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<TTF_Font, Font::FontDeleter>
Font::load(const std::string &path, int size) {
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
    : Base("font"), handle(load(path, size)), path(path), size(size) {}

Engine::Resource::ID Font::key(const std::string &path, int size) {
    return hashKey("Font:" + path + ":" + std::to_string(size));
}

std::string Font::describe() const {
    ::YAML::Node name;
    name["type"] = "Font";
    name["path"] = Engine::Format::path(this->path);
    name["size"] = this->size;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
