#include "text.hpp"

#include "../engine/game.hpp"
#include "../engine/property.hpp"

#include <stdexcept>

void Text::registerType() {
    Node::registerType<Text>("Text");
}

void Text::render(SDL_Renderer *renderer) {
    if (textureDirty) {
        rebuildTexture(renderer);
    }

    if (texture == nullptr) {
        Node::render(renderer);
        return;
    }

    SDL_Rect destination{
            position.x,
            position.y,
            textureSize.x,
            textureSize.y,
    };

    SDL_RenderCopy(renderer, texture, nullptr, &destination);

    Node::render(renderer);
}

void Text::setProperty(const std::string &name, const std::string &value) {
    if (name == "colour") {
        colour = Property::parseColour(value, name);
        textureDirty = true;
        return;
    }

    if (name == "font") {
        fontPath = value;
        textureDirty = true;
        return;
    }

    if (name == "position") {
        position = Property::parsePoint(value, name);
        return;
    }

    if (name == "size") {
        fontSize = Property::parseInteger(value, name);
        textureDirty = true;
        return;
    }

    if (name == "text") {
        text = value;
        textureDirty = true;
        return;
    }

    Node::setProperty(name, value);
}

void Text::rebuildTexture(SDL_Renderer *renderer) {
    texture = nullptr;
    textureSize = SDL_Point{0, 0};
    textureDirty = false;

    if (text.empty()) {
        return;
    }

    if (fontPath.empty()) {
        throw std::runtime_error(
                "Text node is missing required font property.");
    }

    if (fontSize <= 0) {
        throw std::runtime_error(
                "Text node font size must be greater than 0.");
    }

    const auto &textureAsset = Game::getInstance().getAssets().getTextTexture(
            renderer, fontPath, fontSize, colour, text);

    texture = textureAsset.texture;
    textureSize = textureAsset.size;
}
