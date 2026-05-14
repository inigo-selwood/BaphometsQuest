#include "colourChangeBox.hpp"

void ColourChangeBox::registerType() {
    Engine::Node::registerType<ColourChangeBox>("ColourChangeBox");
}

void ColourChangeBox::turnGreen() {
    this->setProperty("colour", SDL_Color{64, 255, 112, 255});
}
