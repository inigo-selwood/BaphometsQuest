#pragma once

#include "../../../engine/nodes/native/map.hpp"
#include "../../../engine/nodes/native/object.hpp"
#include "../../../engine/runtime/game.hpp"
#include "../../../engine/utils/parse.hpp"

#include <SDL.h>

#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

namespace Scenes::Play::Components {

/** Coordinate gameplay between play-scene world children */
class World : public Engine::Nodes::Object {
  public:
    void setup() override {
        Engine::Game &game = this->getGame();
        const std::string currentMap =
            game.state.ensure("current-map", std::string{"home-town"});

        this->getMap()->setProperty("active-map", currentMap);
    }

    /** Move an actor through the world when map traversal allows it */
    bool requestMove(Engine::Nodes::Object &actor, SDL_Point movement) {
        const SDL_Point position = actor.getProperty<SDL_Point>("position");
        const SDL_Point target{
            position.x + movement.x,
            position.y + movement.y,
        };

        if(!this->canMove(position, target)) {
            return false;
        }

        actor.setProperty("position", target);
        this->handleObjectsAt(actor, target);

        return true;
    }

    /** Return true when movement between world-local pixels is permitted */
    bool canMove(SDL_Point fromPixel, SDL_Point toPixel) const {
        const std::shared_ptr<Engine::Nodes::Map> map = this->getMap();
        const SDL_Point mapPosition = map->getProperty<SDL_Point>("position");

        return map->canMove(
            SDL_Point{
                fromPixel.x - mapPosition.x,
                fromPixel.y - mapPosition.y,
            },
            SDL_Point{
                toPixel.x - mapPosition.x,
                toPixel.y - mapPosition.y,
            }
        );
    }

  private:
    /** Return gameplay objects at a world-local pixel position */
    std::vector<Engine::Resource::MapObject>
    findObjectsAt(SDL_Point worldPixel) const {
        const std::shared_ptr<Engine::Nodes::Map> map = this->getMap();
        const SDL_Point mapPosition = map->getProperty<SDL_Point>("position");

        return map->findObjectsAt(
            SDL_Point{
                worldPixel.x - mapPosition.x,
                worldPixel.y - mapPosition.y,
            }
        );
    }

    /** Apply world-owned behaviour for objects at the given position */
    void handleObjectsAt(Engine::Nodes::Object &actor, SDL_Point worldPixel) {
        for(const Engine::Resource::MapObject &object :
            this->findObjectsAt(worldPixel)) {
            if(!this->isTeleport(object)) {
                continue;
            }

            this->handleTeleport(actor, object);
        }
    }

    /** Return true when a map object represents a teleport trigger */
    bool isTeleport(const Engine::Resource::MapObject &object) const {
        return object.type == "teleport"
            || object.properties.contains("teleport")
            || (object.properties.contains("map")
                && object.properties.contains("spawn"));
    }

    /** Store teleport targets in game state and optionally queue a scene */
    void handleTeleport(
        Engine::Nodes::Object &actor,
        const Engine::Resource::MapObject &object
    ) {
        Engine::Game &game = this->getGame();
        const std::string map = this->getProperty(object, "map");
        const std::string spawn = this->getProperty(object, "spawn");
        const std::string scene = this->getProperty(object, "scene");

        if(!map.empty()) {
            game.state.set("current-map", map);
            this->getMap()->setProperty("active-map", map);
        }

        if(!spawn.empty()) {
            const SDL_Point spawnCell = Engine::Parse::point(spawn);
            const int step =
                actor.hasProperty("step") ? actor.getProperty<int>("step") : 8;
            const SDL_Point mapPosition =
                this->getMap()->getProperty<SDL_Point>("position");
            const SDL_Point spawnPosition{
                mapPosition.x + (spawnCell.x * step),
                mapPosition.y + (spawnCell.y * step),
            };

            actor.setProperty("position", spawnPosition);
            game.state.set("player-position", spawnPosition);
        }

        spdlog::debug(
            "Triggered teleport '{}' to map '{}' spawn '{}'",
            object.name,
            map,
            spawn
        );

        if(!scene.empty()) {
            game.queueScene(scene);
        }
    }

    /** Return the required map child */
    std::shared_ptr<Engine::Nodes::Map> getMap() const {
        return this->getChild<Engine::Nodes::Map>("map");
    }

    /** Return an object property or an empty string when absent */
    static std::string getProperty(
        const Engine::Resource::MapObject &object,
        const std::string &name
    ) {
        const auto property = object.properties.find(name);

        if(property == object.properties.end()) {
            return {};
        }

        return property->second;
    }
};

} // namespace Scenes::Play::Components
