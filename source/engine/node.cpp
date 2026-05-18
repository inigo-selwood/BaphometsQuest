#include "node.hpp"

namespace Engine {

Game &Node::getGame() {
    const std::shared_ptr<Game> game = this->game.lock();

    if(game == nullptr) {
        throw std::runtime_error("Node is not attached to a game");
    }

    return *game;
}

const Game &Node::getGame() const {
    const std::shared_ptr<Game> game = this->game.lock();

    if(game == nullptr) {
        throw std::runtime_error("Node is not attached to a game");
    }

    return *game;
}

void Node::addChild(const std::shared_ptr<Node> &child) {
    if(child == nullptr) {
        throw std::runtime_error("Node child must not be null");
    }

    if(child.get() == this) {
        throw std::runtime_error("Node cannot be added as its own child");
    }

    child->parent = this->weak_from_this();
    child->attach(this->game);
    this->children.push_back(child);
}

bool Node::hasProperty(const std::string &name) const {
    return this->properties.contains(name);
}

bool Node::hasHook(Hook hook) const {
    return this->hooks.contains(hook);
}

void Node::enter() {}

void Node::exit() {}

void Node::input(const SDL_Event &) {}

void Node::process(float) {}

void Node::render(SDL_Renderer &) {}

void Node::declareHook(Hook hook) {
    this->hooks.insert(hook);
}

void Node::attach(const std::weak_ptr<Game> &game) {
    this->game = game;

    for(const auto &child : this->children) {
        child->parent = this->weak_from_this();
        child->attach(this->game);
    }
}

} // namespace Engine
