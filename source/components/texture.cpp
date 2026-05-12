#include "texture.hpp"

#include "../engine/game.hpp"
#include "../engine/property.hpp"

#include <stdexcept>

Texture::Texture() {
    setRenderFunction(
            [this](SDL_Renderer *renderer) { renderSelf(renderer); });
}

void Texture::registerType() {
    Node::registerType<Texture>("Texture");
}

void Texture::setProperty(const std::string &name, const std::string &value) {
    if (name == "path") {
        path = value;
        return;
    }

    if (name == "position") {
        position = Property::parsePoint(value, name);
        return;
    }

    if (name == "region") {
        region = Property::parseRect(value, name);
        return;
    }

    Node::setProperty(name, value);
}

SDL_Point Texture::getPosition() const {
    return position;
}

void Texture::renderSelf(SDL_Renderer *renderer) {
    if (path.empty()) {
        throw std::runtime_error("Texture node is missing required path.");
    }

    const auto &textureAsset =
            Game::getInstance().getAssets().getImageTexture(renderer, path);

    const SDL_Rect source = region.w == 0 || region.h == 0
            ? SDL_Rect{0, 0, textureAsset.size.x, textureAsset.size.y}
            : region;
    const SDL_Point position = getGlobalPosition();
    SDL_Rect destination{
            position.x,
            position.y,
            source.w,
            source.h,
    };

    SDL_RenderCopy(renderer, textureAsset.texture, &source, &destination);
}
