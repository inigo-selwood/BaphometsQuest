#pragma once

#include <SDL.h>

namespace Engine::Render {

/** Coordinate mode used by the current render subtree */
enum class CanvasMode {
    Screen,
    World,
};

inline constexpr SDL_Point DEFAULT_ORIGIN{0, 0};
inline constexpr SDL_Point DEFAULT_VIEWPORT_ORIGIN{0, 0};
inline constexpr SDL_Rect DEFAULT_SCREEN_BOUNDS{0, 0, 0, 0};
inline constexpr CanvasMode DEFAULT_CANVAS_MODE = CanvasMode::Screen;

/** Render state inherited while walking the node tree */
struct Context {
    SDL_Point origin = DEFAULT_ORIGIN;
    SDL_Point viewportOrigin = DEFAULT_VIEWPORT_ORIGIN;
    SDL_Rect screenBounds = DEFAULT_SCREEN_BOUNDS;
    CanvasMode mode = DEFAULT_CANVAS_MODE;

    /** Return the viewport origin that centres the camera focus */
    static SDL_Point centreViewportOn(SDL_Point focus, SDL_Rect bounds) {
        return SDL_Point{
            focus.x - (bounds.w / 2),
            focus.y - (bounds.h / 2),
        };
    }

    /** Add a local node offset to the inherited origin */
    void addOrigin(SDL_Point offset) {
        this->origin.x += offset.x;
        this->origin.y += offset.y;
    }

    /** Reset local drawing origin for a new canvas layer */
    void resetOrigin() {
        this->origin = DEFAULT_ORIGIN;
    }
};

} // namespace Engine::Render
