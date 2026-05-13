#include "text.hpp"

#include "../engine/properties/property.hpp"
#include "../engine/runtime/game.hpp"

#include <stdexcept>

Text::Text() {
    this->setRenderFunction([this](SDL_Renderer *renderer) {
        this->renderSelf(renderer);
    });
}

void Text::registerType() {
    Engine::Node::registerType<Text>("Text");
}

void Text::setProperty(const std::string &name, const std::string &value) {
    if(name == "colour") {
        this->colour = Engine::Property::parseColour(value, name);
        this->textureDirty = true;
        return;
    }

    if(name == "font") {
        this->fontPath = value;
        this->textureDirty = true;
        return;
    }

    if(name == "position") {
        this->position = Engine::Property::parsePoint(value, name);
        return;
    }

    if(name == "size") {
        this->fontSize = Engine::Property::parseInteger(value, name);
        this->textureDirty = true;
        return;
    }

    if(name == "text") {
        this->text = value;
        this->textureDirty = true;
        return;
    }

    Engine::Node::setProperty(name, value);
}

SDL_Point Text::getPosition() const {
    return this->position;
}

void Text::renderSelf(SDL_Renderer *renderer) {
    if(this->textureDirty) {
        this->rebuildTexture(renderer);
    }

    if(this->texture == nullptr) {
        return;
    }

    const SDL_Point position = this->getGlobalPosition();
    SDL_Rect destination{
        position.x,
        position.y,
        this->textureSize.x,
        this->textureSize.y,
    };

    SDL_RenderCopy(renderer, this->texture, nullptr, &destination);
}

void Text::rebuildTexture(SDL_Renderer *renderer) {
    this->texture = nullptr;
    this->textureSize = SDL_Point{0, 0};
    this->textureDirty = false;

    if(this->text.empty()) {
        return;
    }

    if(this->fontPath.empty()) {
        throw std::runtime_error(
            "Text node is missing required font property."
        );
    }

    if(this->fontSize <= 0) {
        throw std::runtime_error(
            "Text node font size must be greater than 0."
        );
    }

    const auto &textureAsset =
        Engine::Game::getInstance().getAssets().getTextTexture(
            renderer,
            this->fontPath,
            this->fontSize,
            this->colour,
            this->text
        );

    this->texture = textureAsset.texture;
    this->textureSize = textureAsset.size;
}
