#pragma once

#include "includes.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Node {
  public:
    using Factory = std::function<std::unique_ptr<Node>()>;

    virtual ~Node();

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    template <typename NodeType>
    static void registerType(const std::string &name) {
        static_assert(std::is_base_of_v<Node, NodeType>,
                "NodeType must inherit from Node.");

        spdlog::debug("Registering node '{}'.", name);

        factories()[name] = [] { return std::make_unique<NodeType>(); };
    }

    static std::unique_ptr<Node> create(const std::string &name);

    template <typename NodeType, typename... Arguments>
    NodeType &addChild(const std::string &name, Arguments &&...arguments) {
        static_assert(std::is_base_of_v<Node, NodeType>,
                "NodeType must inherit from Node.");

        if (hasChild(name)) {
            throw std::runtime_error("Node child already exists: " + name);
        }

        auto child = std::make_unique<NodeType>(
                std::forward<Arguments>(arguments)...);
        auto *childReference = child.get();

        addChild(name, std::move(child));

        return *childReference;
    }

    Node &addChild(const std::string &name, std::unique_ptr<Node> child);

    void removeChild(const std::string &name);

    Node &getChild(const std::string &name);
    const Node &getChild(const std::string &name) const;

    template <typename NodeType> NodeType &getChild(const std::string &name) {
        static_assert(std::is_base_of_v<Node, NodeType>,
                "NodeType must inherit from Node.");

        auto *child = dynamic_cast<NodeType *>(&getChild(name));

        if (child == nullptr) {
            throw std::runtime_error("Node child has an unexpected type.");
        }

        return *child;
    }

    template <typename NodeType>
    const NodeType &getChild(const std::string &name) const {
        static_assert(std::is_base_of_v<Node, NodeType>,
                "NodeType must inherit from Node.");

        const auto *child = dynamic_cast<const NodeType *>(&getChild(name));

        if (child == nullptr) {
            throw std::runtime_error("Node child has an unexpected type.");
        }

        return *child;
    }

    bool hasChild(const std::string &name) const;

    std::size_t getChildCount() const;

    bool hasChildren() const;

    const std::string &getName() const;
    const std::string &getTypeName() const;

    void clearChildren();

    void exitTree();
    void enterTree();

    virtual void setProperty(
            const std::string &name, const std::string &value);

    virtual void input(const SDL_Event &event);

    virtual void update(float deltaTime);

    virtual void render(SDL_Renderer *renderer);

  protected:
    Node();

    virtual void onEnterTree();

    virtual void onExitTree();

  private:
    struct Child {
        std::string name;
        std::unique_ptr<Node> node;
    };

    using ChildIterator = std::vector<Child>::iterator;
    using ConstChildIterator = std::vector<Child>::const_iterator;

    static std::unordered_map<std::string, Factory> &factories();

    ChildIterator findChild(const std::string &name);
    ConstChildIterator findChild(const std::string &name) const;

    std::vector<Child> children;
    std::string nodeName = "root";
    std::string nodeTypeName = "Node";
    bool inTree = false;
};
