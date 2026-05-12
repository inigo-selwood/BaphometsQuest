#include "node.hpp"

#include <algorithm>

Node::Node() = default;

Node::~Node() = default;

std::unique_ptr<Node> Node::create(const std::string &name) {
    auto &nodeFactories = factories();

    if (!nodeFactories.contains(name)) {
        throw std::runtime_error("Unknown node type: " + name);
    }

    spdlog::debug("Creating node '{}'.", name);

    auto node = nodeFactories.at(name)();
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

const std::string &Node::getTypeName() const {
    return nodeTypeName;
}

void Node::clearChildren() {
    spdlog::debug("Clearing {} child node(s) from parent '{}'.",
            children.size(),
            getName());

    for (auto &child : children) {
        child.node->exitTree();
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

void Node::input(const SDL_Event &event) {
    for (auto &child : children) {
        child.node->input(event);
    }
}

void Node::setProperty(const std::string &name, const std::string &value) {
    throw std::runtime_error(
            "Unknown property '" + name + "' with value '" + value + "'.");
}

void Node::update(float deltaTime) {
    for (auto &child : children) {
        child.node->update(deltaTime);
    }
}

void Node::render(SDL_Renderer *renderer) {
    for (auto &child : children) {
        child.node->render(renderer);
    }
}

void Node::onEnterTree() {}

void Node::onExitTree() {}

std::unordered_map<std::string, Node::Factory> &Node::factories() {
    static std::unordered_map<std::string, Factory> nodeFactories;
    return nodeFactories;
}

Node::ChildIterator Node::findChild(const std::string &name) {
    return std::ranges::find_if(children,
            [&name](const auto &child) { return child.name == name; });
}

Node::ConstChildIterator Node::findChild(const std::string &name) const {
    return std::ranges::find_if(children,
            [&name](const auto &child) { return child.name == name; });
}
