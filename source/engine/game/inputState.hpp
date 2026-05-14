#pragma once

#include <SDL.h>

#include <array>
#include <cstdint>

namespace Engine {

/** Per-frame input snapshot owned by Game. */
class InputState {
  public:
    /** Clear one-frame pressed/released state before polling events. */
    void beginFrame();

    /** Apply an SDL input event to the current snapshot. */
    void handleEvent(const SDL_Event &event);

    /** Return the current logical mouse position. */
    SDL_Point getMousePosition() const;

    /** Return whether a mouse button is currently held. */
    bool isMouseButtonDown(std::uint8_t button) const;

    /** Return whether a key is currently held. */
    bool isKeyDown(SDL_Scancode key) const;

    /** Return whether a mouse button was pressed this frame. */
    bool wasMouseButtonPressed(std::uint8_t button) const;

    /** Return whether a mouse button was released this frame. */
    bool wasMouseButtonReleased(std::uint8_t button) const;

    /** Return whether a key was pressed this frame. */
    bool wasKeyPressed(SDL_Scancode key) const;

    /** Return whether a key was released this frame. */
    bool wasKeyReleased(SDL_Scancode key) const;

  private:
    static int mouseButtonIndex(std::uint8_t button);

    std::array<bool, SDL_NUM_SCANCODES> keysDown{};
    std::array<bool, SDL_NUM_SCANCODES> keysPressed{};
    std::array<bool, SDL_NUM_SCANCODES> keysReleased{};

    std::array<bool, 8> mouseButtonsDown{};
    std::array<bool, 8> mouseButtonsPressed{};
    std::array<bool, 8> mouseButtonsReleased{};
    SDL_Point mousePosition{0, 0};
};

} // namespace Engine
