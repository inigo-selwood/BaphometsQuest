#include "texture.hpp"

#include "../engine/properties/property.hpp"
#include "../engine/runtime/game.hpp"

#include <stdexcept>

Texture::Texture() {
    this->setRenderFunction([this](SDL_Renderer *renderer) {
        this->renderSelf(renderer);
    });
}

void Texture::registerType() {
    Engine::Node::registerType<Texture>("Texture");
}

void Texture::setProperty(const std::string &name, const std::string &value) {
    if(name == "path") {
        this->path = value;
        return;
    }

    if(name == "position") {
        this->position = Engine::Property::parsePoint(value, name);
        return;
    }

    if(name == "region") {
        this->region = Engine::Property::parseRect(value, name);
        return;
    }

    Engine::Node::setProperty(name, value);
}

SDL_Point Texture::getPosition() const {
    return this->position;
}

void Texture::renderSelf(SDL_Renderer *renderer) {
    if(this->path.empty()) {
        throw std::runtime_error("Texture node is missing required path.");
    }

    const auto &textureAsset =
        Engine::Game::getInstance().getAssets().getImageTexture(
            renderer,
            this->path
        );

    const SDL_Rect source = this->region.w == 0 || this->region.h == 0
        ? SDL_Rect{0, 0, textureAsset.size.x, textureAsset.size.y}
        : this->region;
    const SDL_Point position = this->getGlobalPosition();
    SDL_Rect destination{
        position.x,
        position.y,
        source.w,
        source.h,
    };

    SDL_RenderCopy(renderer, textureAsset.texture, &source, &destination);
}
