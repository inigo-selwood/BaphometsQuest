#include "clickableRedBox.hpp"

ClickableRedBox::ClickableRedBox() {
    setInputFunction([this](const SDL_Event &event) { inputSelf(event); });
}

void ClickableRedBox::registerType() {
    Node::registerType<ClickableRedBox>("ClickableRedBox");
}

void ClickableRedBox::inputSelf(const SDL_Event &event) {
    if (event.type != SDL_MOUSEBUTTONDOWN) {
        return;
    }

    SDL_Point clickPosition{event.button.x, event.button.y};

    if (!isPointInside(clickPosition)) {
        return;
    }

    emitSignal("clicked",
            static_cast<int>(event.button.button),
            clickPosition.x,
            clickPosition.y);
}

void ClickableRedBox::onEnterTree() {
    registerSignal<int, int, int>("clicked");
}
