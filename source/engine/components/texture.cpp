#include "texture.hpp"

#include "../format/format.hpp"
#include "../game/game.hpp"

#include <stdexcept>

Texture::Texture() {
    this->registerProperty<std::string>(
        "path",
        "Texture",
        [this] { return this->path; },
        [this](const std::string &value) {
            this->path = value;
            this->textureID.reset();
        },
        [](const std::string &value) {
            return Engine::Format::filePath(value);
        }
    );
    this->registerProperty<SDL_Rect>(
        "region",
        "Texture",
        [this] { return this->region; },
        [this](const SDL_Rect &value) { this->region = value; }
    );
    this->setRenderFunction([this](SDL_Renderer *renderer) {
        this->renderSelf(renderer);
    });
}

void Texture::registerType() {
    Engine::Node::registerType<Texture>("Texture");
}

void Texture::renderSelf(SDL_Renderer *renderer) {
    if(this->path.empty()) {
        throw std::runtime_error("Texture node is missing required path.");
    }

    auto &assets = Engine::Game::getInstance().getAssets();

    if(!this->textureID.has_value()) {
        this->textureID = assets.loadImageTexture(renderer, this->path);
    }

    auto &texture = assets.get<SDL_Texture>(*this->textureID);
    const SDL_Point textureSize = assets.getTextureSize(*this->textureID);

    const SDL_Rect source = this->region.w == 0 || this->region.h == 0
        ? SDL_Rect{0, 0, textureSize.x, textureSize.y}
        : this->region;
    const SDL_Point position = this->getWorldPosition();
    SDL_Rect destination{
        position.x,
        position.y,
        source.w,
        source.h,
    };

    SDL_RenderCopy(renderer, &texture, &source, &destination);
}
