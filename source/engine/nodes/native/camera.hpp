#pragma once

#include "object.hpp"

namespace Engine::Nodes {

/** Camera node used as the active focus for world canvas rendering */
class Camera : public Engine::Nodes::Object {
  public:
    /** Return true when this camera can be selected for rendering */
    bool isActive() const {
        return Engine::Nodes::Base::isActive();
    }
};

} // namespace Engine::Nodes
