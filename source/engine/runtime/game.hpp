#pragma once

#include "../resource/types/yaml.hpp"
#include "../resource_manager.hpp"
#include "../signal_manager.hpp"
#include "lifecycle.hpp"

#include <filesystem>
#include <memory>
#include <string>

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
    ~Game();

    /** Load settings and start engine-level services */
    void start(
        const std::filesystem::path &executablePath,
        const std::string &consoleLogLevel
    );

    /** Create a window and renderer for the current run stub */
    void run();

    /** Queue the active game loop to stop */
    void queueQuit();

    /** Shared resource manager */
    Engine::Resource::Manager resources;

    /** Shared signal manager */
    Engine::Signal::Manager signals;

    /** Active SDL renderer */
    std::unique_ptr<SDL_Renderer, RendererDeleter> renderer;

    /** Active SDL window */
    std::unique_ptr<SDL_Window, WindowDeleter> window;

  private:

    /** Whether the game loop is currently running */
    bool running = false;

    /** Target frames per second for the game loop */
    int frameRate = 0;
};

} // namespace Engine
