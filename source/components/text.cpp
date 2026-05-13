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

    if(!this->textureID.has_value()) {
        return;
    }

    auto &assets = Engine::Game::getInstance().getAssets();
    auto &texture = assets.get<SDL_Texture>(*this->textureID);
    const SDL_Point textureSize = assets.getTextureSize(*this->textureID);
    const SDL_Point position = this->getGlobalPosition();
    SDL_Rect destination{
        position.x,
        position.y,
        textureSize.x,
        textureSize.y,
    };

    SDL_RenderCopy(renderer, &texture, nullptr, &destination);
}

void Text::rebuildTexture(SDL_Renderer *renderer) {
    this->textureID.reset();
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

    this->textureID = Engine::Game::getInstance().getAssets().loadTextTexture(
        renderer,
        this->fontPath,
        this->fontSize,
        this->colour,
        this->text
    );
}
