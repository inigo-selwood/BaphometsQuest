#pragma once

#include "../../nodes/base.hpp"
#include "../render/context.hpp"

#include <memory>
#include <optional>
#include <vector>

#include <SDL.h>

namespace Engine {

class Game;

namespace Nodes {

/** Dispatcher for the active node tree */
class Manager {
  public:
    explicit Manager(Game &game);

    /** Set the active root node for dispatch */
    void setRoot(const std::shared_ptr<Base> &root);

    /** Dispatch enter hooks for the active tree */
    void enter();

    /** Dispatch exit hooks for the active tree */
    void exit();

    /** Dispatch input hooks for the active tree */
    void input(const SDL_Event &event);

    /** Dispatch process hooks for the active tree */
    void process(float deltaSeconds);

    /** Dispatch render hooks for the active tree */
    void render(SDL_Renderer &renderer);

  private:
    /** Depth-first walk used when inactive nodes still need initialization */
    template <typename Callback>
    void walkAll(const std::shared_ptr<Base> &node, Callback &&callback) {
        if(node == nullptr) {
            return;
        }

        callback(*node);

        for(std::size_t index = 0; index < node->children.size(); index++) {
            this->walkAll(node->children[index], callback);
        }
    }

    /** Depth-first walk used by hook dispatch methods */
    template <typename Callback>
    void walk(const std::shared_ptr<Base> &node, Callback &&callback) {
        if(node == nullptr) {
            return;
        }

        if(!node->active) {
            return;
        }

        callback(*node);

        for(std::size_t index = 0; index < node->children.size(); index++) {
            this->walk(node->children[index], callback);
        }
    }

    /** Snapshot active input nodes before dispatching an event */
    void collectInputNodes(
        const std::shared_ptr<Base> &node,
        std::vector<std::shared_ptr<Base>> &nodes
    ) const;

    /** Find the active world camera before rendering */
    void findActiveCamera(
        const std::shared_ptr<Base> &node,
        Engine::Render::Context context,
        std::optional<SDL_Point> &cameraFocus
    ) const;

    /** Render the active tree with inherited canvas context */
    void renderNode(
        const std::shared_ptr<Base> &node,
        Engine::Render::Context context,
        SDL_Renderer &renderer
    ) const;

    Game &game;
    std::shared_ptr<Base> root;
};

} // namespace Nodes

} // namespace Engine
