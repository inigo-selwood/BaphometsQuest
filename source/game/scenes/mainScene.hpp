#pragma once

#include "../../engine/node/node.hpp"

/** Initial scene loaded from resources/scenes/main.xml. */
class MainScene : public Engine::Node {
  public:
    MainScene() = default;
    ~MainScene() override = default;

  protected:
    void onEnterTree() override;
};
