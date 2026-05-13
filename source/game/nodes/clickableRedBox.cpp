#include "clickableRedBox.hpp"

ClickableRedBox::ClickableRedBox() {
    this->setInputFunction([this](const SDL_Event &event) {
        this->inputSelf(event);
    });
}

void ClickableRedBox::registerType() {
    Engine::Node::registerType<ClickableRedBox>("ClickableRedBox");
}

void ClickableRedBox::inputSelf(const SDL_Event &event) {
    if(event.type != SDL_MOUSEBUTTONDOWN) {
        return;
    }

    SDL_Point clickPosition{event.button.x, event.button.y};

    if(!this->isPointInside(clickPosition)) {
        return;
    }

    this->emitSignal(
        "clicked",
        static_cast<int>(event.button.button),
        clickPosition.x,
        clickPosition.y
    );
}

void ClickableRedBox::onEnterTree() {
    this->registerSignal<int, int, int>("clicked");
}
