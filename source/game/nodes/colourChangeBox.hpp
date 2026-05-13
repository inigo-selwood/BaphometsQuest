#pragma once

#include "../../components/box.hpp"

/** Demo box that can change colour in response to gameplay signals. */
class ColourChangeBox : public Box {
  public:
    ColourChangeBox() = default;

    /** Register the ColourChangeBox XML node type. */
    static void registerType();

    /** Change the box fill colour to green. */
    void turnGreen();
};
