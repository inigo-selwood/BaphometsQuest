#include "node.hpp"

#include <algorithm>
#include <utility>

Node::Node() = default;

Node::~Node() = default;

std::unique_ptr<Node> Node::create(const std::string &name) {
    auto &factories = nodeFactories();

    if (!factories.contains(name)) {
        throw std::runtime_error("Unknown node type: " + name);
    }

    spdlog::debug("Creating node '{}'.", name);

    auto node = factories.at(name)();
    node->nodeTypeName = name;

    return node;
}

Node &Node::addChild(const std::string &name, std::unique_ptr<Node> child) {
    if (child == nullptr) {
        throw std::runtime_error("Cannot add null child node: " + name);
    }

    if (hasChild(name)) {
        throw std::runtime_error("Node child already exists: " + name);
    }

    auto &childReference = *child;
    childReference.nodeName = name;
    childReference.parent = this;

    spdlog::debug("Adding child node '{}' of type '{}' to parent '{}'.",
            name,
            childReference.getTypeName(),
            getName());

    children.push_back({name, std::move(child)});

    if (inTree) {
        childReference.enterTree();
    }

    return childReference;
}

void Node::removeChild(const std::string &name) {
    const auto childIterator = findChild(name);

    if (childIterator == children.end()) {
        spdlog::warn(
                "Tried to remove unknown child node '{}' from parent '{}'.",
                name,
                getName());
        return;
    }

    spdlog::debug(
            "Removing child node '{}' from parent '{}'.", name, getName());
    childIterator->node->exitTree();
    childIterator->node->parent = nullptr;
    children.erase(childIterator);
}

Node &Node::getChild(const std::string &name) {
    const auto childIterator = findChild(name);

    if (childIterator == children.end()) {
        throw std::runtime_error("Unknown node child: " + name);
    }

    return *childIterator->node;
}

const Node &Node::getChild(const std::string &name) const {
    const auto childIterator = findChild(name);

    if (childIterator == children.end()) {
        throw std::runtime_error("Unknown node child: " + name);
    }

    return *childIterator->node;
}

bool Node::hasChild(const std::string &name) const {
    return findChild(name) != children.end();
}

std::size_t Node::getChildCount() const {
    return children.size();
}

bool Node::hasChildren() const {
    return !children.empty();
}

const std::string &Node::getName() const {
    return nodeName;
}

Node *Node::getParent() {
    return parent;
}

const Node *Node::getParent() const {
    return parent;
}

SDL_Point Node::getGlobalPosition() const {
    const SDL_Point parentPosition =
            parent == nullptr ? SDL_Point{0, 0} : parent->getGlobalPosition();
    const SDL_Point position = getPosition();

    return SDL_Point{
            parentPosition.x + position.x,
            parentPosition.y + position.y,
    };
}

const std::string &Node::getTypeName() const {
    return nodeTypeName;
}

void Node::clearChildren() {
    spdlog::debug("Clearing {} child node(s) from parent '{}'.",
            children.size(),
            getName());

    for (auto &child : children) {
        child.node->exitTree();
        child.node->parent = nullptr;
    }

    children.clear();
}

void Node::enterTree() {
    if (inTree) {
        return;
    }

    inTree = true;
    spdlog::trace("Node entered tree.");
    onEnterTree();

    for (auto &child : children) {
        child.node->enterTree();
    }
}

void Node::exitTree() {
    if (!inTree) {
        return;
    }

    for (auto &child : children) {
        child.node->exitTree();
    }

    onExitTree();
    spdlog::trace("Node exited tree.");
    inTree = false;
}

bool Node::hasSignal(const std::string &name) const {
    return signals.contains(name);
}

void Node::input(const SDL_Event &event) {
    if (inputFunction) {
        inputFunction(event);
    }

    for (auto &child : children) {
        child.node->input(event);
    }
}

void Node::setProperty(const std::string &name, const std::string &value) {
    throw std::runtime_error(
            "Unknown property '" + name + "' with value '" + value + "'.");
}

void Node::process(float deltaTime) {
    if (processFunction) {
        processFunction(deltaTime);
    }

    for (auto &child : children) {
        child.node->process(deltaTime);
    }
}

void Node::render(SDL_Renderer *renderer) {
    if (renderFunction) {
        renderFunction(renderer);
    }

    for (auto &child : children) {
        child.node->render(renderer);
    }
}

SDL_Point Node::getPosition() const {
    return SDL_Point{0, 0};
}

void Node::onEnterTree() {}

void Node::onExitTree() {}

void Node::setInputFunction(InputFunction newInputFunction) {
    inputFunction = std::move(newInputFunction);
}

void Node::setProcessFunction(ProcessFunction newProcessFunction) {
    processFunction = std::move(newProcessFunction);
}

void Node::setRenderFunction(RenderFunction newRenderFunction) {
    renderFunction = std::move(newRenderFunction);
}

std::unordered_map<std::string, Node::NodeFactory> &Node::nodeFactories() {
    static std::unordered_map<std::string, NodeFactory> factories;
    return factories;
}

Node::ChildIterator Node::findChild(const std::string &name) {
    return std::ranges::find_if(children,
            [&name](const auto &child) { return child.name == name; });
}

Node::ConstChildIterator Node::findChild(const std::string &name) const {
    return std::ranges::find_if(children,
            [&name](const auto &child) { return child.name == name; });
}

Node::Signal &Node::getSignal(const std::string &name) {
    if (!hasSignal(name)) {
        throw std::runtime_error(
                "Unknown signal '" + name + "' on node '" + getName() + "'.");
    }

    return signals.at(name);
}

const Node::Signal &Node::getSignal(const std::string &name) const {
    if (!hasSignal(name)) {
        throw std::runtime_error(
                "Unknown signal '" + name + "' on node '" + getName() + "'.");
    }

    return signals.at(name);
}
