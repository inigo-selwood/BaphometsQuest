#pragma once

#include "../engine/scene.hpp"

class MainScene : public Scene {
  public:
    MainScene() = default;
    ~MainScene() override = default;

  protected:
    void onEnterTree() override;
};
