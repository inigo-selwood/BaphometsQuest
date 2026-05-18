#include "base.hpp"

#include <atomic>

#include <spdlog/spdlog.h>

namespace Engine::Nodes {

namespace {

std::atomic<std::uint32_t> nextID = 1;

} // namespace

Base::Base() : id(generateID()) {}

Game &Base::getGame() {
    const std::shared_ptr<Game> game = this->game.lock();

    if(game == nullptr) {
        throw std::runtime_error("Node is not attached to a game");
    }

    return *game;
}

const Game &Base::getGame() const {
    const std::shared_ptr<Game> game = this->game.lock();

    if(game == nullptr) {
        throw std::runtime_error("Node is not attached to a game");
    }

    return *game;
}

void Base::addChild(
    const std::string &name,
    const std::shared_ptr<Base> &child
) {
    if(child == nullptr) {
        throw std::runtime_error("Node child must not be null");
    }

    if(child.get() == this) {
        throw std::runtime_error("Node cannot be added as its own child");
    }

    child->name = name;
    child->parent = this->weak_from_this();
    child->attach(this->game);
    this->children.push_back(child);

    spdlog::debug(
        "Added node {} to parent {}",
        child->describe(),
        this->describe()
    );
}

bool Base::hasProperty(const std::string &name) const {
    return this->properties.contains(name);
}

bool Base::hasHook(Hook hook) const {
    return this->hooks.contains(hook);
}

void Base::enter() {}

void Base::exit() {}

void Base::input(const SDL_Event &) {}

void Base::process(float) {}

void Base::render(SDL_Renderer &) {}

void Base::declareHook(Hook hook) {
    this->hooks.insert(hook);
}

void Base::attach(const std::weak_ptr<Game> &game) {
    this->game = game;

    for(const auto &child : this->children) {
        child->parent = this->weak_from_this();
        child->attach(this->game);
    }
}

std::uint32_t Base::generateID() {
    return nextID++;
}

} // namespace Engine::Nodes
