#pragma once

#include "node.hpp"

namespace Engine {

/** Root node type for a playable scene. */
class Scene : public Node {
  public:
    ~Scene() override = default;

  protected:
    Scene() = default;
};

} // namespace Engine
