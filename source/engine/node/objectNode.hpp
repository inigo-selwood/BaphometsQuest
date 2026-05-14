#pragma once

#include "graphicNode.hpp"

namespace Engine {

/** Graphic node with world-position helpers for gameplay objects. */
class ObjectNode : public GraphicNode {
  public:
    /** Return the node position accumulated through its parents. */
    SDL_Point getWorldPosition() const;
};

} // namespace Engine
