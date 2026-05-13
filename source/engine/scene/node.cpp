#include "node.hpp"

#include "nodeRegistry.hpp"

#include <algorithm>
#include <utility>

namespace Engine {

Node::Node() = default;

Node::~Node() = default;

std::unique_ptr<Node> Node::create(const std::string &name) {
    return NodeRegistry::getInstance().create(name);
}

Node &Node::addChild(const std::string &name, std::unique_ptr<Node> child) {
    if(child == nullptr) {
        throw std::runtime_error("Cannot add null child node: " + name);
    }

    if(this->hasChild(name)) {
        throw std::runtime_error("Node child already exists: " + name);
    }

    auto &childReference = *child;
    childReference.nodeName = name;
    childReference.parent = this;

    spdlog::debug(
        "Adding child node '{}' of type '{}' to parent '{}'.",
        name,
        childReference.getTypeName(),
        this->getName()
    );

    this->children.push_back({name, std::move(child)});

    if(this->inTree) {
        childReference.enterTree();
    }

    return childReference;
}

void Node::removeChild(const std::string &name) {
    const auto childIterator = this->findChild(name);

    if(childIterator == this->children.end()) {
        spdlog::warn(
            "Tried to remove unknown child node '{}' from parent '{}'.",
            name,
            this->getName()
        );
        return;
    }

    spdlog::debug(
        "Removing child node '{}' from parent '{}'.",
        name,
        this->getName()
    );
    childIterator->node->exitTree();
    childIterator->node->parent = nullptr;
    this->children.erase(childIterator);
}

Node &Node::getChild(const std::string &name) {
    const auto childIterator = this->findChild(name);

    if(childIterator == this->children.end()) {
        throw std::runtime_error("Unknown node child: " + name);
    }

    return *childIterator->node;
}

const Node &Node::getChild(const std::string &name) const {
    const auto childIterator = this->findChild(name);

    if(childIterator == this->children.end()) {
        throw std::runtime_error("Unknown node child: " + name);
    }

    return *childIterator->node;
}

bool Node::hasChild(const std::string &name) const {
    return this->findChild(name) != this->children.end();
}

std::size_t Node::getChildCount() const {
    return this->children.size();
}

bool Node::hasChildren() const {
    return !this->children.empty();
}

const std::string &Node::getName() const {
    return this->nodeName;
}

Node *Node::getParent() {
    return this->parent;
}

const Node *Node::getParent() const {
    return this->parent;
}

SDL_Point Node::getGlobalPosition() const {
    const SDL_Point parentPosition = this->parent == nullptr
        ? SDL_Point{0, 0}
        : this->parent->getGlobalPosition();
    const SDL_Point position = this->getPosition();

    return SDL_Point{
        parentPosition.x + position.x,
        parentPosition.y + position.y,
    };
}

const std::string &Node::getTypeName() const {
    return this->nodeTypeName;
}

void Node::clearChildren() {
    spdlog::debug(
        "Clearing {} child node(s) from parent '{}'.",
        this->children.size(),
        this->getName()
    );

    for(auto &child : this->children) {
        child.node->exitTree();
        child.node->parent = nullptr;
    }

    this->children.clear();
}

void Node::enterTree() {
    if(this->inTree) {
        return;
    }

    this->inTree = true;
    spdlog::trace("Node entered tree.");
    this->onEnterTree();

    for(auto &child : this->children) {
        child.node->enterTree();
    }
}

void Node::exitTree() {
    if(!this->inTree) {
        return;
    }

    for(auto &child : this->children) {
        child.node->exitTree();
    }

    this->onExitTree();
    spdlog::trace("Node exited tree.");
    this->inTree = false;
}

bool Node::hasSignal(const std::string &name) const {
    return this->signals.has(name);
}

void Node::input(const SDL_Event &event) {
    if(this->inputFunction) {
        this->inputFunction(event);
    }

    for(auto &child : this->children) {
        child.node->input(event);
    }
}

void Node::setProperty(const std::string &name, const std::string &value) {
    throw std::runtime_error(
        "Unknown property '" + name + "' with value '" + value + "'."
    );
}

void Node::process(float deltaTime) {
    if(this->processFunction) {
        this->processFunction(deltaTime);
    }

    for(auto &child : this->children) {
        child.node->process(deltaTime);
    }
}

void Node::render(SDL_Renderer *renderer) {
    if(this->renderFunction) {
        this->renderFunction(renderer);
    }

    for(auto &child : this->children) {
        child.node->render(renderer);
    }
}

SDL_Point Node::getPosition() const {
    return SDL_Point{0, 0};
}

void Node::onEnterTree() {}

void Node::onExitTree() {}

void Node::setInputFunction(InputFunction newInputFunction) {
    this->inputFunction = std::move(newInputFunction);
}

void Node::setProcessFunction(ProcessFunction newProcessFunction) {
    this->processFunction = std::move(newProcessFunction);
}

void Node::setRenderFunction(RenderFunction newRenderFunction) {
    this->renderFunction = std::move(newRenderFunction);
}

void Node::registerNodeType(
    const std::string &name,
    std::function<std::unique_ptr<Node>()> factory
) {
    NodeRegistry::getInstance().registerType(name, std::move(factory));
}

Node::ChildIterator Node::findChild(const std::string &name) {
    return std::ranges::find_if(this->children, [&name](const auto &child) {
        return child.name == name;
    });
}

Node::ConstChildIterator Node::findChild(const std::string &name) const {
    return std::ranges::find_if(this->children, [&name](const auto &child) {
        return child.name == name;
    });
}

} // namespace Engine
