#pragma once

#include "../components/box.hpp"

class ColourChangeBox : public Box {
  public:
    ColourChangeBox() = default;

    static void registerType();

    void turnGreen();
};
