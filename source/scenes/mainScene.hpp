#pragma once

#include "../engine/scene/scene.hpp"

/** Initial scene loaded from resources/scenes/main.xml. */
class MainScene : public Engine::Scene {
  public:
    MainScene() = default;
    ~MainScene() override = default;

  protected:
    void onEnterTree() override;
};
