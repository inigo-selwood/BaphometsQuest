#include "inputState.hpp"

#include <algorithm>

namespace Engine {

void InputState::beginFrame() {
    std::fill(this->keysPressed.begin(), this->keysPressed.end(), false);
    std::fill(this->keysReleased.begin(), this->keysReleased.end(), false);
    std::fill(
        this->mouseButtonsPressed.begin(),
        this->mouseButtonsPressed.end(),
        false
    );
    std::fill(
        this->mouseButtonsReleased.begin(),
        this->mouseButtonsReleased.end(),
        false
    );
}

SDL_Point InputState::getMousePosition() const {
    return this->mousePosition;
}

void InputState::handleEvent(const SDL_Event &event) {
    switch(event.type) {
    case SDL_KEYDOWN:
        if(!event.key.repeat) {
            this->keysDown[event.key.keysym.scancode] = true;
            this->keysPressed[event.key.keysym.scancode] = true;
        }
        break;
    case SDL_KEYUP:
        this->keysDown[event.key.keysym.scancode] = false;
        this->keysReleased[event.key.keysym.scancode] = true;
        break;
    case SDL_MOUSEBUTTONDOWN: {
        const int index = InputState::mouseButtonIndex(event.button.button);
        if(index >= 0) {
            this->mouseButtonsDown[index] = true;
            this->mouseButtonsPressed[index] = true;
        }
        this->mousePosition = SDL_Point{event.button.x, event.button.y};
        break;
    }
    case SDL_MOUSEBUTTONUP: {
        const int index = InputState::mouseButtonIndex(event.button.button);
        if(index >= 0) {
            this->mouseButtonsDown[index] = false;
            this->mouseButtonsReleased[index] = true;
        }
        this->mousePosition = SDL_Point{event.button.x, event.button.y};
        break;
    }
    case SDL_MOUSEMOTION:
        this->mousePosition = SDL_Point{event.motion.x, event.motion.y};
        break;
    default:
        break;
    }
}

bool InputState::isKeyDown(SDL_Scancode key) const {
    return this->keysDown[key];
}

bool InputState::isMouseButtonDown(std::uint8_t button) const {
    const int index = InputState::mouseButtonIndex(button);
    return index >= 0 && this->mouseButtonsDown[index];
}

int InputState::mouseButtonIndex(std::uint8_t button) {
    const int index = static_cast<int>(button) - 1;

    if(index < 0 || index >= 8) {
        return -1;
    }

    return index;
}

bool InputState::wasKeyPressed(SDL_Scancode key) const {
    return this->keysPressed[key];
}

bool InputState::wasKeyReleased(SDL_Scancode key) const {
    return this->keysReleased[key];
}

bool InputState::wasMouseButtonPressed(std::uint8_t button) const {
    const int index = InputState::mouseButtonIndex(button);
    return index >= 0 && this->mouseButtonsPressed[index];
}

bool InputState::wasMouseButtonReleased(std::uint8_t button) const {
    const int index = InputState::mouseButtonIndex(button);
    return index >= 0 && this->mouseButtonsReleased[index];
}

} // namespace Engine
