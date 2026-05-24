#include "base.hpp"

#include "../utils/parse.hpp"
#include "native/label.hpp"

#include <atomic>
#include <string_view>
#include <typeindex>

#include <spdlog/spdlog.h>

namespace Engine::Nodes {

namespace {

std::atomic<std::uint32_t> nextID = 1;

Engine::Nodes::Label::Justification parseJustification(std::string_view text) {
    if(text == "left") {
        return Engine::Nodes::Label::Justification::Left;
    }

    if(text == "centre") {
        return Engine::Nodes::Label::Justification::Centre;
    }

    if(text == "right") {
        return Engine::Nodes::Label::Justification::Right;
    }

    throw std::runtime_error(
        "Justification value must be left, centre, or right: '"
        + std::string(text) + "'"
    );
}

} // namespace

Base::Base() : id(generateID()) {
    this->declareProperty("active", this->active);
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
    if(name.empty()) {
        throw std::runtime_error("Child path must not be empty");
    }

    std::string_view path{name};
    std::shared_ptr<Base> child;
    const Base *parent = this;
    std::size_t offset = 0;
    const auto getDirectChild = [](const Base &parent,
                                    std::string_view childName,
                                    const std::string &path) {
        if(childName.empty()) {
            throw std::runtime_error(
                "Child path '" + path + "' has empty segment"
            );
        }

        for(const auto &child : parent.children) {
            if(child->name == childName) {
                return child;
            }
        }

        throw std::runtime_error(
            "Unknown child node '" + std::string(childName) + "' in path '"
            + path + "'"
        );
    };

    while(offset <= path.size()) {
        const std::size_t separator = path.find('.', offset);
        const std::size_t end =
            separator == std::string_view::npos ? path.size() : separator;
        child =
            getDirectChild(*parent, path.substr(offset, end - offset), name);

        if(separator == std::string_view::npos) {
            return child;
        }

        parent = child.get();
        offset = separator + 1;
    }

    throw std::runtime_error(
        "Child path '" + name + "' could not be resolved"
    );
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
        this->setProperty(name, parseJustification(value));
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

void Base::render(Engine::Render::Canvas &) {}

void Base::applyRenderContext(Engine::Render::Context &) const {}

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
