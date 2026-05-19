#include "base.hpp"

#include "../utils/parse.hpp"
#include "native/label.hpp"

#include <atomic>
#include <typeindex>

#include <spdlog/spdlog.h>

namespace Engine::Nodes {

namespace {

std::atomic<std::uint32_t> nextID = 1;

} // namespace

Base::Base() : id(generateID()) {
    this->declareProperty("visible", this->visible);
}

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

std::shared_ptr<Base> Base::getChild(const std::string &name) const {
    for(const auto &child : this->children) {
        if(child->name == name) {
            return child;
        }
    }

    throw std::runtime_error("Unknown child node '" + name + "'");
}

const std::vector<std::shared_ptr<Base>> &Base::getChildren() const {
    return this->children;
}

bool Base::hasProperty(const std::string &name) const {
    return this->properties.contains(name);
}

bool Base::hasHook(Hook hook) const {
    return this->hooks.contains(hook);
}

void Base::setPropertyFromText(
    const std::string &name,
    const std::string &value
) {
    const auto property = this->properties.find(name);

    if(property == this->properties.end()) {
        throw std::runtime_error("Unknown node property '" + name + "'");
    }

    const std::type_index &type = property->second.type;

    if(type == std::type_index(typeid(std::string))) {
        this->setProperty(name, value);
    } else if(type == std::type_index(typeid(int))) {
        this->setProperty(name, Engine::Parse::integer(value));
    } else if(type == std::type_index(typeid(bool))) {
        this->setProperty(name, Engine::Parse::boolean(value));
    } else if(type == std::type_index(typeid(SDL_Point))) {
        this->setProperty(name, Engine::Parse::point(value));
    } else if(type == std::type_index(typeid(SDL_Rect))) {
        this->setProperty(name, Engine::Parse::rect(value));
    } else if(type == std::type_index(typeid(SDL_Color))) {
        this->setProperty(name, Engine::Parse::colour(value));
    } else if(type
        == std::type_index(typeid(Engine::Nodes::Label::Justification))) {
        this->setProperty(name, Engine::Parse::justification(value));
    } else {
        throw std::runtime_error(
            "Node property '" + name + "' cannot be set from text"
        );
    }
}

void Base::runSetup() {
    if(this->setupComplete) {
        return;
    }

    this->setupComplete = true;
    this->setup();
}

void Base::setup() {}

void Base::enter() {}

void Base::exit() {}

void Base::input(const SDL_Event &) {}

void Base::process(float) {}

void Base::render(SDL_Renderer &) {}

bool Base::loadXmlChildren(const tinyxml2::XMLElement &) {
    return false;
}

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
