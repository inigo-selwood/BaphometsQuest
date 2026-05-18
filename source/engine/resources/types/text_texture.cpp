#include "text_texture.hpp"

#include "../../utils/format.hpp"

#include <sstream>
#include <stdexcept>

namespace Engine::Resource {

namespace {

struct SurfaceDeleter {
    void operator()(SDL_Surface *surface) const {
        SDL_FreeSurface(surface);
    }
};

} // namespace

std::unique_ptr<SDL_Texture, TextTexture::TextureDeleter> TextTexture::render(
    SDL_Renderer *renderer,
    const Engine::Resource::Font &font,
    SDL_Color colour,
    const std::string &text
) {
    std::unique_ptr<SDL_Surface, SurfaceDeleter> surface(
        TTF_RenderUTF8_Blended(font.handle.get(), text.c_str(), colour)
    );

    if(!surface) {
        throw std::runtime_error(
            "Failed to render text texture '" + text + "': " + TTF_GetError()
        );
    }

    std::unique_ptr<SDL_Texture, TextureDeleter> texture(
        SDL_CreateTextureFromSurface(renderer, surface.get())
    );

    if(!texture) {
        throw std::runtime_error(
            "Failed to create text texture '" + text + "': " + SDL_GetError()
        );
    }

    return texture;
}

SDL_Rect
TextTexture::querySize(SDL_Texture *texture, const std::string &text) {
    SDL_Rect size{0, 0, 0, 0};

    if(SDL_QueryTexture(texture, nullptr, nullptr, &size.w, &size.h) != 0) {
        throw std::runtime_error(
            "Failed to query text texture '" + text + "': " + SDL_GetError()
        );
    }

    return size;
}

TextTexture::TextTexture(
    SDL_Renderer *renderer,
    Engine::Resource::ID fontID,
    const Engine::Resource::Font &font,
    SDL_Color colour,
    const std::string &text
)
    : handle(render(renderer, font, colour, text)), fontID(fontID),
      colour(colour), size(querySize(this->handle.get(), text)), text(text) {}

std::string TextTexture::key(
    SDL_Renderer *renderer,
    Engine::Resource::ID fontID,
    SDL_Color colour,
    const std::string &text
) {
    std::ostringstream stream;
    stream << "TextTexture:" << renderer << ":" << fontID << ":"
           << Engine::Format::colour(colour) << ":" << text;

    return stream.str();
}

std::string TextTexture::describe() const {
    ::YAML::Node name;
    name["type"] = "TextTexture";
    name["font-id"] = static_cast<std::uint64_t>(this->fontID);
    name["colour"] = Engine::Format::colour(this->colour);
    name["text"] = this->text;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
