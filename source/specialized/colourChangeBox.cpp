#include "colourChangeBox.hpp"

void ColourChangeBox::registerType() {
    Node::registerType<ColourChangeBox>("ColourChangeBox");
}

void ColourChangeBox::turnGreen() {
    setColour(SDL_Color{64, 255, 112, 255});
}
