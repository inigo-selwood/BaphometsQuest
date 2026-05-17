#pragma once

#include "signal_manager.hpp"
#include "resources/manager.hpp"

#include <memory>

#include <SDL.h>

namespace Engine {

/** Service root for engine-level managers */
class Game {
  private:
    struct RendererDeleter {
        void operator()(SDL_Renderer *renderer) const {
            SDL_DestroyRenderer(renderer);
        }
    };

    struct WindowDeleter {
        void operator()(SDL_Window *window) const {
            SDL_DestroyWindow(window);
        }
    };

  public:
    /** Create a window and renderer for the current run stub */
    void run();

    /** Shared resource manager */
    Engine::Resource::Manager Resources;

    /** Shared signal manager */
    Engine::Signal::Manager Signals;

    /** Active SDL renderer */
    std::unique_ptr<SDL_Renderer, RendererDeleter> Renderer;

    /** Active SDL window */
    std::unique_ptr<SDL_Window, WindowDeleter> Window;
};

} // namespace Engine
