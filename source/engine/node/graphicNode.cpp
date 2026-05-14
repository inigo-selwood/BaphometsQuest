#include "graphicNode.hpp"

namespace Engine {

GraphicNode::GraphicNode() {
    this->registerProperty<SDL_Point>(
        "position",
        "GraphicNode",
        [this] { return this->position; },
        [this](const SDL_Point &value) { this->position = value; }
    );
    this->registerProperty<SDL_Point>(
        "size",
        "GraphicNode",
        [this] { return this->size.value_or(SDL_Point{0, 0}); },
        [this](const SDL_Point &value) { this->size = value; }
    );
    this->registerProperty<bool>(
        "visible",
        "GraphicNode",
        [this] { return this->visible; },
        [this](const bool &value) { this->visible = value; }
    );
}

std::optional<SDL_Point> GraphicNode::getSize() const {
    return this->size;
}

bool GraphicNode::isVisible() const {
    return this->visible;
}

SDL_Point GraphicNode::getPosition() const {
    return this->position;
}

void GraphicNode::render(SDL_Renderer *renderer) {
    if(!this->visible) {
        return;
    }

    Node::render(renderer);
}

} // namespace Engine
