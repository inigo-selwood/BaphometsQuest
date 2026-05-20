#pragma once

#include "../utils/format.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <SDL.h>
#include <spdlog/spdlog.h>

namespace tinyxml2 {
class XMLElement;
}

namespace Engine {

class Game;

namespace Render {
class Canvas;
struct Context;
} // namespace Render

namespace Nodes {

class Manager;

/** Game-loop hooks that a node can opt into */
enum class Hook {
    Enter,
    Exit,
    Input,
    Process,
    Render,
};

/** Base class for objects that can participate in the game tree */
class Base : public std::enable_shared_from_this<Base> {
  public:
    Base();
    virtual ~Base() = default;

    const std::uint32_t id;

    /** Return the game this node is attached to */
    Game &getGame();

    /** Return the game this node is attached to */
    const Game &getGame() const;

    /** Add a child node to this node */
    void addChild(const std::string &name, const std::shared_ptr<Base> &child);

    /** Return a named direct child */
    std::shared_ptr<Base> getChild(const std::string &name) const;

    /** Return direct children in tree order */
    const std::vector<std::shared_ptr<Base>> &getChildren() const;

    /** Return a named direct child with a checked node type */
    template <typename NodeType>
    std::shared_ptr<NodeType> getChild(const std::string &name) const {
        static_assert(
            std::is_base_of_v<Base, NodeType>,
            "NodeType must inherit from Engine::Nodes::Base"
        );

        std::shared_ptr<NodeType> child =
            std::dynamic_pointer_cast<NodeType>(this->getChild(name));

        if(child == nullptr) {
            throw std::runtime_error(
                "Child node '" + name + "' type does not match"
            );
        }

        return child;
    }

    /** Return true when this node has declared a property */
    bool hasProperty(const std::string &name) const;

    /** Return true when this node has declared a hook */
    bool hasHook(Hook hook) const;

    /** Return a read-only reference to a typed property */
    template <typename Value>
    const Value &getProperty(const std::string &name) const {
        const auto property = this->properties.find(name);

        if(property == this->properties.end()) {
            throw std::runtime_error("Unknown node property '" + name + "'");
        }

        using StoredValue = std::decay_t<Value>;

        if(property->second.type != std::type_index(typeid(StoredValue))) {
            throw std::runtime_error(
                "Node property '" + name + "' type does not match"
            );
        }

        return *static_cast<const StoredValue *>(property->second.value);
    }

    /**
     * Update a typed property through direct assignment or its declared setter
     *
     * Callback-backed properties are responsible for assigning their wrapped
     * member so they can rebuild dependent state at the same time
     */
    template <typename Value>
    void setProperty(const std::string &name, const Value &value) {
        const auto property = this->properties.find(name);

        if(property == this->properties.end()) {
            throw std::runtime_error("Unknown node property '" + name + "'");
        }

        using StoredValue = std::decay_t<Value>;

        if(property->second.type != std::type_index(typeid(StoredValue))) {
            throw std::runtime_error(
                "Node property '" + name + "' type does not match"
            );
        }

        if(property->second.setter) {
            property->second.setter(&value);
        } else {
            *static_cast<StoredValue *>(property->second.value) = value;
        }

        spdlog::trace(
            "Set property {}.{} to {}",
            this->describePropertyOwner(),
            name,
            Engine::Format::propertyValue(name, value)
        );
    }

    /** Update a typed property from text using its declared property type */
    void
    setPropertyFromText(const std::string &name, const std::string &value);

    /** Run setup once for this node */
    void runSetup();

    virtual void setup();
    virtual void enter();
    virtual void exit();
    virtual void input(const SDL_Event &event);
    virtual void process(float deltaSeconds);
    virtual void render(Engine::Render::Canvas &canvas);

    /** Apply this node's transform to the inherited render context */
    virtual void applyRenderContext(Engine::Render::Context &context) const;

    /** Return true when this node consumed its own XML child elements */
    virtual bool loadXmlChildren(const tinyxml2::XMLElement &element);

  protected:
    /** Declare a property backed by direct member assignment */
    template <typename Value>
    void declareProperty(const std::string &name, Value &member) {
        if(this->hasProperty(name)) {
            throw std::runtime_error(
                "Node property '" + name + "' already exists"
            );
        }

        using StoredValue = std::decay_t<Value>;

        this->properties.emplace(
            name,
            Property{std::type_index(typeid(StoredValue)), &member}
        );
    }

    /**
     * Declare a property that delegates assignment to a typed setter callback
     *
     * The member is still registered so getProperty() can return the current
     * value after the callback has performed its update
     */
    template <typename Value, typename Callback>
    void declareProperty(
        const std::string &name,
        Value &member,
        Callback &&callback
    ) {
        if(this->hasProperty(name)) {
            throw std::runtime_error(
                "Node property '" + name + "' already exists"
            );
        }

        using StoredValue = std::decay_t<Value>;

        this->properties.emplace(
            name,
            Property{
                std::type_index(typeid(StoredValue)),
                &member,
                [callback = std::forward<Callback>(callback)](
                    const void *value
                ) mutable {
                    callback(*static_cast<const StoredValue *>(value));
                },
            }
        );
    }

    /** Declare that this node participates in a game-loop hook */
    void declareHook(Hook hook);

  private:
    friend class Manager;

    struct Property {
        std::type_index type;
        void *value;
        std::function<void(const void *)> setter;
    };

    /** Attach this node and its descendants to a game context */
    void attach(const std::weak_ptr<Game> &game);

    /** Return the compact node label used in trace logs */
    std::string describe() const {
        if(this->name.empty()) {
            return "#" + std::to_string(this->id);
        }

        return "'" + this->name + "' #" + std::to_string(this->id);
    }

    /** Return the compact node label used in property logs */
    std::string describePropertyOwner() const {
        if(this->name.empty()) {
            return "#" + std::to_string(this->id);
        }

        return this->name;
    }

    static std::uint32_t generateID();

    std::string name;
    std::vector<std::shared_ptr<Base>> children;
    std::weak_ptr<Base> parent;
    std::weak_ptr<Game> game;
    bool visible = true;
    bool setupComplete = false;
    std::unordered_map<std::string, Property> properties;
    std::set<Hook> hooks;
};

} // namespace Nodes

} // namespace Engine
