#include "TextTexture.hpp"

#include "../../utils/format.hpp"

#include <stdexcept>

namespace Engine::Resource {

namespace {

struct SurfaceDeleter {
    void operator()(SDL_Surface *surface) const {
        SDL_FreeSurface(surface);
    }
};

} // namespace

TextTexture::TextTexture(
    SDL_Renderer *renderer,
    const Engine::Resource::Font &font,
    SDL_Color colour,
    const std::string &text
)
    : fontDescription(font.describe()), colour(colour), text(text) {
    std::unique_ptr<SDL_Surface, SurfaceDeleter> surface(
        TTF_RenderUTF8_Blended(font.getHandle(), text.c_str(), colour)
    );

    if(!surface) {
        throw std::runtime_error(
            "Failed to render text texture '" + text + "': " + TTF_GetError()
        );
    }

    this->size = {0, 0, surface->w, surface->h};
    this->texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));

    if(!this->texture) {
        throw std::runtime_error(
            "Failed to create text texture '" + text + "': " + SDL_GetError()
        );
    }
}

SDL_Texture *TextTexture::getHandle() const {
    return this->texture.get();
}

std::string TextTexture::describe() const {
    ::YAML::Node name;
    name["type"] = "TextTexture";
    name["font"] = this->fontDescription;
    name["colour"] = Engine::Format::colour(this->colour);
    name["text"] = this->text;

    return this->formatDescription(name);
}

SDL_Rect TextTexture::getSize() const {
    return this->size;
}

void TextTexture::TextureDeleter::operator()(SDL_Texture *texture) const {
    SDL_DestroyTexture(texture);
}

} // namespace Engine::Resource
