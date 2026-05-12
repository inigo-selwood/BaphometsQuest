#include "box.hpp"

#include "../engine/property.hpp"

Box::Box() {
    setRenderFunction(
            [this](SDL_Renderer *renderer) { renderSelf(renderer); });
}

void Box::registerType() {
    Node::registerType<Box>("Box");
}

void Box::setColour(SDL_Color newColour) {
    colour = newColour;
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

SDL_Point Box::getPosition() const {
    return position;
}

bool Box::isPointInside(SDL_Point point) const {
    const SDL_Point globalPosition = getGlobalPosition();

    return point.x >= globalPosition.x && point.x < globalPosition.x + size.x
            && point.y >= globalPosition.y
            && point.y < globalPosition.y + size.y;
}

void Box::renderSelf(SDL_Renderer *renderer) {
    const SDL_Point globalPosition = getGlobalPosition();
    SDL_Rect rectangle{globalPosition.x, globalPosition.y, size.x, size.y};

    SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
    SDL_RenderFillRect(renderer, &rectangle);
}
