#include "box.hpp"

Box::Box() {
    this->registerProperty<SDL_Color>(
        "colour",
        "Box",
        [this] { return this->colour; },
        [this](const SDL_Color &value) { this->colour = value; }
    );
    this->setRenderFunction([this](SDL_Renderer *renderer) {
        this->renderSelf(renderer);
    });
}

void Box::registerType() {
    Engine::Node::registerType<Box>("Box");
}

bool Box::isPointInside(SDL_Point point) const {
    const SDL_Point size = this->getSize().value_or(SDL_Point{0, 0});
    const SDL_Point worldPosition = this->getWorldPosition();

    return point.x >= worldPosition.x && point.x < worldPosition.x + size.x
        && point.y >= worldPosition.y && point.y < worldPosition.y + size.y;
}

void Box::renderSelf(SDL_Renderer *renderer) {
    const SDL_Point size = this->getSize().value_or(SDL_Point{0, 0});
    const SDL_Point worldPosition = this->getWorldPosition();
    SDL_Rect rectangle{worldPosition.x, worldPosition.y, size.x, size.y};

    SDL_SetRenderDrawColor(
        renderer,
        this->colour.r,
        this->colour.g,
        this->colour.b,
        this->colour.a
    );
    SDL_RenderFillRect(renderer, &rectangle);
}
