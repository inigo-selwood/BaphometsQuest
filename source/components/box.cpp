#include "box.hpp"

#include "../engine/properties/property.hpp"

Box::Box() {
    this->setRenderFunction([this](SDL_Renderer *renderer) {
        this->renderSelf(renderer);
    });
}

void Box::registerType() {
    Engine::Node::registerType<Box>("Box");
}

void Box::setColour(SDL_Color newColour) {
    this->colour = newColour;
}

void Box::setProperty(const std::string &name, const std::string &value) {
    if(name == "colour") {
        this->colour = Engine::Property::parseColour(value, name);
        return;
    }

    if(name == "size") {
        this->size = Engine::Property::parsePoint(value, name);
        return;
    }

    if(name == "position") {
        this->position = Engine::Property::parsePoint(value, name);
        return;
    }

    Engine::Node::setProperty(name, value);
}

SDL_Point Box::getPosition() const {
    return this->position;
}

bool Box::isPointInside(SDL_Point point) const {
    const SDL_Point globalPosition = this->getGlobalPosition();

    return point.x >= globalPosition.x
        && point.x < globalPosition.x + this->size.x
        && point.y >= globalPosition.y
        && point.y < globalPosition.y + this->size.y;
}

void Box::renderSelf(SDL_Renderer *renderer) {
    const SDL_Point globalPosition = this->getGlobalPosition();
    SDL_Rect rectangle{globalPosition.x,
        globalPosition.y,
        this->size.x,
        this->size.y};

    SDL_SetRenderDrawColor(
        renderer,
        this->colour.r,
        this->colour.g,
        this->colour.b,
        this->colour.a
    );
    SDL_RenderFillRect(renderer, &rectangle);
}
