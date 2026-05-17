#include "image_texture.hpp"

#include <SDL_image.h>

#include <stdexcept>

namespace Engine::Resource {

std::unique_ptr<SDL_Texture, ImageTexture::TextureDeleter>
ImageTexture::loadTexture(SDL_Renderer *renderer, const std::string &path) {
    std::unique_ptr<SDL_Texture, TextureDeleter> texture(
        IMG_LoadTexture(renderer, path.c_str())
    );

    if(!texture) {
        throw std::runtime_error(
            "Failed to load image texture '" + path + "': " + IMG_GetError()
        );
    }

    return texture;
}

SDL_Rect
ImageTexture::queryTextureSize(SDL_Texture *texture, const std::string &path) {
    SDL_Rect size{0, 0, 0, 0};

    if(SDL_QueryTexture(texture, nullptr, nullptr, &size.w, &size.h) != 0) {
        throw std::runtime_error(
            "Failed to query image texture '" + path + "': " + SDL_GetError()
        );
    }

    return size;
}

ImageTexture::ImageTexture(SDL_Renderer *renderer, const std::string &path)
    : Handle(loadTexture(renderer, path)), Path(path),
      Size(queryTextureSize(this->Handle.get(), path)) {}

std::string ImageTexture::describe() const {
    ::YAML::Node name;
    name["type"] = "ImageTexture";
    name["path"] = this->Path;

    return this->formatDescription(name);
}

} // namespace Engine::Resource
