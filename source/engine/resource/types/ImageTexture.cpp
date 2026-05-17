#include "ImageTexture.hpp"

#include <SDL_image.h>

#include <stdexcept>

namespace Engine::Resource {

ImageTexture::ImageTexture(SDL_Renderer *renderer, const std::string &path)
    : texture(IMG_LoadTexture(renderer, path.c_str())), path(path) {
    if(!this->texture) {
        throw std::runtime_error(
            "Failed to load image texture '" + path + "': " + IMG_GetError()
        );
    }

    if(SDL_QueryTexture(
           this->texture.get(),
           nullptr,
           nullptr,
           &this->size.w,
           &this->size.h
       ) != 0) {
        throw std::runtime_error(
            "Failed to query image texture '" + path + "': " + SDL_GetError()
        );
    }
}

SDL_Texture *ImageTexture::getHandle() const {
    return this->texture.get();
}

std::string ImageTexture::describe() const {
    ::YAML::Node name;
    name["type"] = "ImageTexture";
    name["path"] = this->path;

    return this->formatDescription(name);
}

SDL_Rect ImageTexture::getSize() const {
    return this->size;
}

void ImageTexture::TextureDeleter::operator()(SDL_Texture *texture) const {
    SDL_DestroyTexture(texture);
}

} // namespace Engine::Resource
