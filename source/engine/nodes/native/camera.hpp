#pragma once

#include "object.hpp"

namespace Engine::Nodes {

/** Camera node used as the active focus for world canvas rendering */
class Camera : public Engine::Nodes::Object {
  public:
    Camera() {
        this->declareProperty("active", this->active);
    }

    /** Return true when this camera can be selected for rendering */
    bool isActive() const {
        return this->active;
    }

  private:
    bool active = true;
};

} // namespace Engine::Nodes
