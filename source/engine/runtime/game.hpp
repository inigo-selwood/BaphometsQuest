#pragma once

#include "../node.hpp"
#include "../node_manager.hpp"
#include "../resource/types/yaml.hpp"
#include "../resource_manager.hpp"
#include "../signal_manager.hpp"
#include "lifecycle.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <SDL.h>

namespace Engine {

/** Service root for engine-level managers */
class Game : public std::enable_shared_from_this<Game> {
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
    Game();
    ~Game();

    /** Load settings and start engine-level services */
    void start(
        const std::filesystem::path &executablePath,
        const std::string &consoleLogLevel
    );

    /** Run the active game loop */
    void run();

    /** Queue the active game loop to stop */
    void queueQuit();

    template <typename SceneType>
    void registerScene(const std::string &name) {
        static_assert(
            std::is_base_of_v<Engine::Node, SceneType>,
            "SceneType must inherit from Engine::Node."
        );

        if(this->sceneFactories.contains(name)) {
            throw std::runtime_error(
                "Scene '" + name + "' is already registered"
            );
        }

        this->sceneFactories[name] = []() {
            return std::make_shared<SceneType>();
        };
    }

    /** Queue a scene switch for the next frame boundary */
    void queueScene(const std::string &name);

    /** Shared resource manager */
    Engine::Resource::Manager resources;

    /** Shared signal manager */
    Engine::Signal::Manager signals;

    /** Active node tree manager */
    Engine::NodeManager nodeManager;

    /** Current active scene root */
    std::shared_ptr<Engine::Node> currentScene;

    /** Active SDL renderer */
    std::unique_ptr<SDL_Renderer, RendererDeleter> renderer;

    /** Active SDL window */
    std::unique_ptr<SDL_Window, WindowDeleter> window;

  private:
    /** Whether the game loop is currently running */
    bool running = false;

    /** Target frames per second for the game loop */
    int frameRate = 0;

    std::unordered_map<
        std::string,
        std::function<std::shared_ptr<Engine::Node>()>>
        sceneFactories;
    std::optional<std::string> queuedScene;
};

} // namespace Engine
