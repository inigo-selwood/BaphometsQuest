#include "box.hpp"

#include "../engine/property.hpp"

void Box::registerType() {
    Node::registerType<Box>("Box");
}

void Box::setProperty(const std::string &name, const std::string &value) {
    if (name == "colour") {
        colour = Property::parseColour(value, name);
        return;
    }

    if (name == "size") {
        size = Property::parsePoint(value, name);
        return;
    }

    if (name == "position") {
        position = Property::parsePoint(value, name);
        return;
    }

    Node::setProperty(name, value);
}

void Box::render(SDL_Renderer *renderer) {
    SDL_Rect rectangle{position.x, position.y, size.x, size.y};

    SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
    SDL_RenderFillRect(renderer, &rectangle);

    Node::render(renderer);
}
