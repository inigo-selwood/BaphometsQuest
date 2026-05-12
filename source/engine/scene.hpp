#pragma once

#include "node.hpp"

class Scene : public Node {
  public:
    ~Scene() override = default;

  protected:
    Scene() = default;
};
