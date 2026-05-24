#pragma once

#include "../../../engine/nodes/native/map.hpp"
#include "../../../engine/nodes/native/object.hpp"

#include <SDL.h>

#include <memory>
#include <string>
#include <vector>

namespace Scenes::Play::Components {

/** Coordinate gameplay between play-scene world children */
class World : public Engine::Nodes::Object {
  public:
    /** Result of asking the world to handle interaction at a point */
    struct Interaction {
        enum class Type {
            None,
            Teleport,
            Dialogue,
            Inspect,
        };

        Type type = Type::None;
        std::string text;
    };

    void setup() override;

    /** Move an actor through the world when map traversal allows it */
    bool requestMove(Engine::Nodes::Object &actor, SDL_Point movement);

    /** Handle the best available interaction at a world-local pixel */
    Interaction interactAt(Engine::Nodes::Object &actor, SDL_Point worldPixel);

    /** Return true when movement between world-local pixels is permitted */
    bool canMove(SDL_Point fromPixel, SDL_Point toPixel) const;

  private:
    /** Return the required map child */
    std::shared_ptr<Engine::Nodes::Map> getMap() const;

    /** Return an object property or an empty string when absent */
    static std::string getProperty(
        const Engine::Resource::MapObject &object,
        const std::string &name
    );

    /** Return gameplay objects at a world-local pixel position */
    std::vector<Engine::Resource::MapObject>
    findObjectsAt(SDL_Point worldPixel) const;

    /** Apply world-owned behaviour for objects at the given position */
    void handleObjectsAt(Engine::Nodes::Object &actor, SDL_Point worldPixel);

    /** Return true when a map object represents a teleport trigger */
    bool isTeleport(const Engine::Resource::MapObject &object) const;

    /** Return a text interaction when the object exposes readable text */
    Interaction getTextInteraction(
        const Engine::Resource::MapObject &object
    ) const;

    /** Store teleport targets in game state and optionally queue a scene */
    void handleTeleport(
        Engine::Nodes::Object &actor,
        const Engine::Resource::MapObject &object
    );
};

} // namespace Scenes::Play::Components
