#pragma once

#include "../../engine/components/box.hpp"

/** Demo box that emits clicked(int button, int x, int y) when clicked. */
class ClickableRedBox : public Box {
  public:
    ClickableRedBox();

    /** Register the ClickableRedBox XML node type. */
    static void registerType();

  private:
    void inputSelf(const SDL_Event &event);
    void onEnterTree() override;
};
