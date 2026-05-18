#pragma once

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

namespace Engine {

class Game;

namespace Nodes {

class Manager;

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

    /** Return true when this node has declared a property */
    bool hasProperty(const std::string &name) const;

    /** Return true when this node has declared a hook */
    bool hasHook(Hook hook) const;

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

        spdlog::trace("Set node property '{}' on {}", name, this->describe());
    }

    virtual void enter();
    virtual void exit();
    virtual void input(const SDL_Event &event);
    virtual void process(float deltaSeconds);
    virtual void render(SDL_Renderer &renderer);

  protected:
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

    void attach(const std::weak_ptr<Game> &game);
    std::string describe() const {
        if(this->name.empty()) {
            return "#" + std::to_string(this->id);
        }

        return "'" + this->name + "' #" + std::to_string(this->id);
    }

    static std::uint32_t generateID();

    std::string name;
    std::vector<std::shared_ptr<Base>> children;
    std::weak_ptr<Base> parent;
    std::weak_ptr<Game> game;
    std::unordered_map<std::string, Property> properties;
    std::set<Hook> hooks;
};

} // namespace Nodes

} // namespace Engine
