#pragma once

#include "../components/box.hpp"

class ClickableRedBox : public Box {
  public:
    ClickableRedBox();

    static void registerType();

  private:
    void inputSelf(const SDL_Event &event);
    void onEnterTree() override;
};
