#pragma once

#include "../../nodes/base.hpp"

#include <memory>

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
    /** Depth-first walk used by all hook dispatch methods */
    template <typename Callback>
    void walk(const std::shared_ptr<Base> &node, Callback &&callback) {
        if(node == nullptr) {
            return;
        }

        if(!node->visible) {
            return;
        }

        callback(*node);

        for(const auto &child : node->children) {
            this->walk(child, callback);
        }
    }

    Game &game;
    std::shared_ptr<Base> root;
};

} // namespace Nodes

} // namespace Engine
