#pragma once

#include "../format/format.hpp"
#include "../parse/parse.hpp"
#include "../signals/signalRegistry.hpp"

#include <SDL.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

namespace Engine {

template <typename ValueType> struct PropertyParser {
    static ValueType parse(const std::string &, const std::string &) = delete;
};

template <> struct PropertyParser<bool> {
    static bool parse(const std::string &value, const std::string &name) {
        return Parse::boolean(value, name);
    }
};

template <> struct PropertyParser<float> {
    static float parse(const std::string &value, const std::string &name) {
        return Parse::floating(value, name);
    }
};

template <> struct PropertyParser<int> {
    static int parse(const std::string &value, const std::string &name) {
        return Parse::integer(value, name);
    }
};

template <> struct PropertyParser<SDL_Color> {
    static SDL_Color parse(const std::string &value, const std::string &name) {
        return Parse::colour(value, name);
    }
};

template <> struct PropertyParser<SDL_Point> {
    static SDL_Point parse(const std::string &value, const std::string &name) {
        return Parse::point(value, name);
    }
};

template <> struct PropertyParser<SDL_Rect> {
    static SDL_Rect parse(const std::string &value, const std::string &name) {
        return Parse::rect(value, name);
    }
};

template <> struct PropertyParser<std::string> {
    static std::string parse(const std::string &value, const std::string &) {
        return value;
    }
};

template <typename ValueType>
std::string propertyValueToString(const ValueType &value) {
    return Format::value(value);
}

template <> inline std::string propertyValueToString<bool>(const bool &value) {
    return Format::boolean(value);
}

template <>
inline std::string propertyValueToString<SDL_Color>(const SDL_Color &value) {
    return Format::colour(value);
}

template <>
inline std::string propertyValueToString<SDL_Point>(const SDL_Point &value) {
    return Format::point(value);
}

template <>
inline std::string propertyValueToString<SDL_Rect>(const SDL_Rect &value) {
    return Format::rect(value);
}

/**
 * Base type for all objects that can live in the scene tree.
 *
 * Nodes own named child nodes, receive lifecycle/input/process/render
 * traversal, expose typed properties, and provide typed local signals.
 */
class Node {
    struct PropertyBindingBase;
    template <typename ValueType> struct PropertyBinding;

  public:
    /** Callback invoked when this node receives an SDL input event. */
    using InputFunction = std::function<void(const SDL_Event &event)>;

    /** Callback invoked once per frame with elapsed seconds. */
    using ProcessFunction = std::function<void(float deltaTime)>;

    /** Callback invoked during render traversal. */
    using RenderFunction = std::function<void(SDL_Renderer *renderer)>;

    struct PropertyInfo {
        std::string name;
        std::string owner;
        std::type_index type;
    };

    virtual ~Node();

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    /**
     * Register a concrete node class for XML scene loading.
     *
     * @tparam NodeType Concrete type deriving from Node.
     * @param name XML element/type name used to create the node.
     */
    template <typename NodeType>
    static void registerType(const std::string &name) {
        static_assert(
            std::is_base_of_v<Node, NodeType>,
            "NodeType must inherit from Node."
        );

        registerNodeType(name, [] { return std::make_unique<NodeType>(); });
    }

    /**
     * Create a registered node type by name.
     *
     * @throws std::runtime_error if the type name is unknown.
     */
    static std::unique_ptr<Node> create(const std::string &name);

    /**
     * Construct and add a named child node.
     *
     * @throws std::runtime_error if the child name already exists.
     */
    template <typename NodeType, typename... Arguments>
    NodeType &addChild(const std::string &name, Arguments &&...arguments) {
        static_assert(
            std::is_base_of_v<Node, NodeType>,
            "NodeType must inherit from Node."
        );

        if(this->hasChild(name)) {
            throw std::runtime_error("Node child already exists: " + name);
        }

        auto child =
            std::make_unique<NodeType>(std::forward<Arguments>(arguments)...);
        auto *childReference = child.get();

        this->addChild(name, std::move(child));

        return *childReference;
    }

    /**
     * Add an already-created child node.
     *
     * The child receives this node as its parent and enters the tree
     * immediately if this node is already inside the tree.
     *
     * @throws std::runtime_error if child is null or the name already exists.
     */
    Node &addChild(const std::string &name, std::unique_ptr<Node> child);

    /** Remove a child by name, exiting it from the tree first if needed. */
    void removeChild(const std::string &name);

    /** Fetch a child node by name. */
    Node &getChild(const std::string &name);

    /** Fetch a child node by name. */
    const Node &getChild(const std::string &name) const;

    /**
     * Fetch a child node by name and cast it to the expected type.
     *
     * @throws std::runtime_error if the child is missing or has another type.
     */
    template <typename NodeType> NodeType &getChild(const std::string &name) {
        static_assert(
            std::is_base_of_v<Node, NodeType>,
            "NodeType must inherit from Node."
        );

        auto *child = dynamic_cast<NodeType *>(&this->getChild(name));

        if(child == nullptr) {
            throw std::runtime_error("Node child has an unexpected type.");
        }

        return *child;
    }

    /**
     * Fetch a child node by name and cast it to the expected type.
     *
     * @throws std::runtime_error if the child is missing or has another type.
     */
    template <typename NodeType>
    const NodeType &getChild(const std::string &name) const {
        static_assert(
            std::is_base_of_v<Node, NodeType>,
            "NodeType must inherit from Node."
        );

        const auto *child =
            dynamic_cast<const NodeType *>(&this->getChild(name));

        if(child == nullptr) {
            throw std::runtime_error("Node child has an unexpected type.");
        }

        return *child;
    }

    /** Return true when a child with this name exists. */
    bool hasChild(const std::string &name) const;

    /** Return the number of direct children. */
    std::size_t getChildCount() const;

    /** Return true when this node has at least one direct child. */
    bool hasChildren() const;

    /** Return the registered property metadata for this node instance. */
    std::vector<PropertyInfo> getProperties() const;

    /** Return the node's scene-tree name. */
    const std::string &getName() const;

    /** Return this node's parent, or nullptr for the tree root. */
    Node *getParent();

    /** Return this node's parent, or nullptr for the tree root. */
    const Node *getParent() const;

    /** Return this node's position accumulated through its parents. */
    SDL_Point getGlobalPosition() const;

    /** Return the registered node type name. */
    const std::string &getTypeName() const;

    /** Return a typed property value. */
    template <typename ValueType>
    ValueType getProperty(const std::string &name) const {
        const auto &property = this->getPropertyBinding(name);

        if(property.type != std::type_index(typeid(ValueType))) {
            throw std::runtime_error(
                "Property '" + name + "' on node '" + this->getName()
                + "' has an unexpected type."
            );
        }

        const auto &typedProperty =
            static_cast<const PropertyBinding<ValueType> &>(property);
        return typedProperty.getter();
    }

    /** Exit and remove all direct children. */
    void clearChildren();

    /**
     * Connect a callback to a registered signal.
     *
     * @tparam Arguments Signal argument types. Must match registration.
     * @throws std::runtime_error if the signal is unknown or mismatched.
     */
    template <typename... Arguments, typename Callback>
    void connectSignal(const std::string &name, Callback &&callback) {
        this->signals.connect<Arguments...>(
            name,
            this->getName(),
            std::forward<Callback>(callback)
        );
    }

    /**
     * Emit a registered signal with typed arguments.
     *
     * @throws std::runtime_error if the signal is unknown or mismatched.
     */
    template <typename... Arguments>
    void emitSignal(const std::string &name, Arguments &&...arguments) {
        this->signals.emit(
            name,
            this->getName(),
            std::forward<Arguments>(arguments)...
        );
    }

    /** Return true when this node has a signal with this name. */
    bool hasSignal(const std::string &name) const;

    /**
     * Register a typed signal on this node.
     *
     * Re-registering with the same signature is allowed.
     *
     * @throws std::runtime_error if an existing signal has another signature.
     */
    template <typename... Arguments>
    void registerSignal(const std::string &name) {
        this->signals.registerSignal<Arguments...>(name, this->getName());
    }

    /** Exit this node and its descendants from the scene tree. */
    void exitTree();

    /** Enter this node and its descendants into the scene tree. */
    void enterTree();

    /** Set a typed property value. */
    template <typename ValueType>
    void setProperty(const std::string &name, const ValueType &value) {
        auto &property = this->getPropertyBinding(name);

        if(property.type != std::type_index(typeid(ValueType))) {
            throw std::runtime_error(
                "Property '" + name + "' on node '" + this->getName()
                + "' has an unexpected type."
            );
        }

        auto &typedProperty =
            static_cast<PropertyBinding<ValueType> &>(property);
        typedProperty.setter(value);
        spdlog::trace(
            "set property {}.{}: to {}",
            this->getName(),
            name,
            typedProperty.valueToString()
        );
    }

    /** Traverse input handling for this node and its children. */
    virtual void input(const SDL_Event &event);

    /** Traverse per-frame processing for this node and its children. */
    virtual void process(float deltaTime);

    /** Traverse rendering for this node and its children. */
    virtual void render(SDL_Renderer *renderer);

  protected:
    Node();

    /** Return the node's local position for global-position accumulation. */
    virtual SDL_Point getPosition() const;

    /** Hook called when this node enters the scene tree. */
    virtual void onEnterTree();

    /** Hook called when this node exits the scene tree. */
    virtual void onExitTree();

    /** Assign this node's optional input callback. */
    void setInputFunction(InputFunction newInputFunction);

    template <typename ValueType>
    void registerProperty(
        const std::string &name,
        const std::string &owner,
        std::function<ValueType(const std::string &, const std::string &)>
            parser,
        std::function<ValueType()> getter,
        std::function<void(const ValueType &)> setter,
        std::function<std::string(const ValueType &)> formatter =
            propertyValueToString<ValueType>
    ) {
        if(this->properties.contains(name)) {
            throw std::runtime_error(
                "Property '" + name + "' is already registered on node '"
                + this->getName() + "'."
            );
        }

        this->properties.emplace(
            name,
            std::make_unique<PropertyBinding<ValueType>>(
                name,
                owner,
                std::move(parser),
                std::move(getter),
                std::move(setter),
                std::move(formatter)
            )
        );
    }

    template <typename ValueType>
    void registerProperty(
        const std::string &name,
        const std::string &owner,
        std::function<ValueType()> getter,
        std::function<void(const ValueType &)> setter
    ) {
        this->registerProperty<ValueType>(
            name,
            owner,
            PropertyParser<ValueType>::parse,
            std::move(getter),
            std::move(setter)
        );
    }

    template <typename ValueType>
    void registerProperty(
        const std::string &name,
        const std::string &owner,
        std::function<ValueType()> getter,
        std::function<void(const ValueType &)> setter,
        std::function<std::string(const ValueType &)> formatter
    ) {
        this->registerProperty<ValueType>(
            name,
            owner,
            PropertyParser<ValueType>::parse,
            std::move(getter),
            std::move(setter),
            std::move(formatter)
        );
    }

    /** Assign this node's optional process callback. */
    void setProcessFunction(ProcessFunction newProcessFunction);

    /** Assign this node's optional render callback. */
    void setRenderFunction(RenderFunction newRenderFunction);

  private:
    struct PropertyBindingBase {
        PropertyBindingBase(
            std::string propertyName,
            std::string propertyOwner,
            std::type_index propertyType
        )
            : name(std::move(propertyName)), owner(std::move(propertyOwner)),
              type(propertyType) {}

        virtual ~PropertyBindingBase() = default;

        virtual void setFromString(const std::string &value) = 0;
        virtual std::string valueToString() const = 0;

        std::string name;
        std::string owner;
        std::type_index type;
    };

    template <typename ValueType>
    struct PropertyBinding final : PropertyBindingBase {
        PropertyBinding(
            const std::string &propertyName,
            const std::string &propertyOwner,
            std::function<ValueType(const std::string &, const std::string &)>
                propertyParser,
            std::function<ValueType()> propertyGetter,
            std::function<void(const ValueType &)> propertySetter,
            std::function<std::string(const ValueType &)> propertyFormatter
        )
            : PropertyBindingBase(
                  propertyName,
                  propertyOwner,
                  std::type_index(typeid(ValueType))
              ),
              formatter(std::move(propertyFormatter)),
              getter(std::move(propertyGetter)),
              parser(std::move(propertyParser)),
              setter(std::move(propertySetter)) {}

        void setFromString(const std::string &value) override {
            this->setter(this->parser(value, this->name));
        }

        std::string valueToString() const override {
            return this->formatter(this->getter());
        }

        std::function<std::string(const ValueType &)> formatter;
        std::function<ValueType()> getter;
        std::function<ValueType(const std::string &, const std::string &)>
            parser;
        std::function<void(const ValueType &)> setter;
    };

    struct Child {
        std::string name;
        std::unique_ptr<Node> node;
    };

    using ChildIterator = std::vector<Child>::iterator;
    using ConstChildIterator = std::vector<Child>::const_iterator;

    static void registerNodeType(
        const std::string &name,
        std::function<std::unique_ptr<Node>()> factory
    );

    ChildIterator findChild(const std::string &name);
    ConstChildIterator findChild(const std::string &name) const;
    PropertyBindingBase &getPropertyBinding(const std::string &name);
    const PropertyBindingBase &
    getPropertyBinding(const std::string &name) const;
    void
    setPropertyFromString(const std::string &name, const std::string &value);

    std::vector<Child> children;

    InputFunction inputFunction;
    ProcessFunction processFunction;
    RenderFunction renderFunction;

    std::string nodeName = "root";
    std::string nodeTypeName = "Node";

    Node *parent = nullptr;
    std::unordered_map<std::string, std::unique_ptr<PropertyBindingBase>>
        properties;
    SignalRegistry signals;
    bool inTree = false;

    friend class NodeRegistry;
    friend void setNodeNameForLoading(Node &node, const std::string &name);
    friend void setNodePropertyFromString(
        Node &node,
        const std::string &name,
        const std::string &value
    );
};

void setNodeNameForLoading(Node &node, const std::string &name);

void setNodePropertyFromString(
    Node &node,
    const std::string &name,
    const std::string &value
);

} // namespace Engine
