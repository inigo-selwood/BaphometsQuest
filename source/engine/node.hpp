#pragma once

#include "includes.hpp"

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

class Node {
  public:
    using InputFunction = std::function<void(const SDL_Event &event)>;
    using NodeFactory = std::function<std::unique_ptr<Node>()>;
    using ProcessFunction = std::function<void(float deltaTime)>;
    using RenderFunction = std::function<void(SDL_Renderer *renderer)>;

    virtual ~Node();

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    template <typename NodeType>
    static void registerType(const std::string &name) {
        static_assert(std::is_base_of_v<Node, NodeType>,
                "NodeType must inherit from Node.");

        spdlog::debug("Registering node '{}'.", name);

        nodeFactories()[name] = [] { return std::make_unique<NodeType>(); };
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
    Node *getParent();
    const Node *getParent() const;
    SDL_Point getGlobalPosition() const;
    const std::string &getTypeName() const;

    void clearChildren();

    template <typename... Arguments, typename Callback>
    void connectSignal(const std::string &name, Callback &&callback) {
        auto &signal = getSignal(name);
        validateSignalArguments<Arguments...>(name, signal);

        spdlog::debug(
                "Connecting to signal '{}' on node '{}'.", name, getName());

        std::function<void(std::decay_t<Arguments>...)> signalCallback =
                std::forward<Callback>(callback);

        signal.callbacks.push_back(
                [signalCallback = std::move(signalCallback)](
                        const auto &arguments) {
                    callSignalCallback<Arguments...>(signalCallback,
                            arguments,
                            std::index_sequence_for<Arguments...>{});
                });
    }

    template <typename... Arguments>
    void emitSignal(const std::string &name, Arguments &&...arguments) {
        auto &signal = getSignal(name);
        validateSignalArguments<Arguments...>(name, signal);

        spdlog::debug("Node '{}' emitted signal '{}' with {} argument(s).",
                getName(),
                name,
                sizeof...(Arguments));

        std::vector<std::any> signalArguments;
        signalArguments.reserve(sizeof...(Arguments));
        (signalArguments.emplace_back(std::forward<Arguments>(arguments)),
                ...);

        for (const auto &signalCallback : signal.callbacks) {
            signalCallback(signalArguments);
        }
    }

    bool hasSignal(const std::string &name) const;

    template <typename... Arguments>
    void registerSignal(const std::string &name) {
        if (hasSignal(name)) {
            validateSignalArguments<Arguments...>(name, signals.at(name));
            return;
        }

        spdlog::debug(
                "Registering signal '{}' on node '{}'.", name, getName());
        signals[name] = Signal{
                {std::type_index(typeid(std::decay_t<Arguments>))...},
                {},
        };
    }

    void exitTree();
    void enterTree();

    virtual void setProperty(
            const std::string &name, const std::string &value);

    virtual void input(const SDL_Event &event);

    virtual void process(float deltaTime);

    virtual void render(SDL_Renderer *renderer);

  protected:
    Node();

    virtual SDL_Point getPosition() const;

    virtual void onEnterTree();

    virtual void onExitTree();

    void setInputFunction(InputFunction newInputFunction);
    void setProcessFunction(ProcessFunction newProcessFunction);
    void setRenderFunction(RenderFunction newRenderFunction);

  private:
    struct Child {
        std::string name;
        std::unique_ptr<Node> node;
    };

    struct Signal {
        std::vector<std::type_index> argumentTypes;
        std::vector<std::function<void(const std::vector<std::any> &)>>
                callbacks;
    };

    using ChildIterator = std::vector<Child>::iterator;
    using ConstChildIterator = std::vector<Child>::const_iterator;

    static std::unordered_map<std::string, NodeFactory> &nodeFactories();

    template <typename... Arguments, std::size_t... Indexes>
    static void callSignalCallback(
            const std::function<void(std::decay_t<Arguments>...)>
                    &signalCallback,
            const std::vector<std::any> &arguments,
            std::index_sequence<Indexes...>) {
        signalCallback(std::any_cast<std::decay_t<Arguments>>(
                arguments.at(Indexes))...);
    }

    template <typename... Arguments>
    void validateSignalArguments(
            const std::string &name, const Signal &signal) const {
        const std::vector<std::type_index> argumentTypes{
                std::type_index(typeid(std::decay_t<Arguments>))...};

        if (signal.argumentTypes == argumentTypes) {
            return;
        }

        throw std::runtime_error("Signal '" + name
                + "' argument types do not match on node '" + getName()
                + "'.");
    }

    ChildIterator findChild(const std::string &name);
    ConstChildIterator findChild(const std::string &name) const;
    Signal &getSignal(const std::string &name);
    const Signal &getSignal(const std::string &name) const;

    std::vector<Child> children;

    InputFunction inputFunction;
    ProcessFunction processFunction;
    RenderFunction renderFunction;

    std::string nodeName = "root";
    std::string nodeTypeName = "Node";

    Node *parent = nullptr;
    std::unordered_map<std::string, Signal> signals;
    bool inTree = false;
};
