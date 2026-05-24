#pragma once

#include "../nodes/base.hpp"
#include "../resources/types/yaml.hpp"
#include "file_watcher.hpp"
#include "lifecycle.hpp"
#include "managers/node.hpp"
#include "managers/resource.hpp"
#include "managers/signal.hpp"
#include "state.hpp"
#include "timer.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

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

    /** Run the active scene loop until a quit event or queueQuit() stops it */
    void run();

    /** Queue the active game loop to stop */
    void queueQuit();

    /** Save persistent game state to the configured save file */
    void saveState() const;

    /** Return the logical screen area currently used for rendering */
    SDL_Rect getScreenSize() const;

    /** Register a default-constructible scene root factory */
    template <typename SceneType> void registerScene(const std::string &name) {
        static_assert(
            std::is_base_of_v<Engine::Nodes::Base, SceneType>,
            "SceneType must inherit from Engine::Nodes::Base."
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

    /** Shared state store */
    Engine::State::Store state;

    /** Active SDL renderer */
    std::unique_ptr<SDL_Renderer, RendererDeleter> renderer;

    /** Active SDL window */
    std::unique_ptr<SDL_Window, WindowDeleter> window;

    /** Active node tree manager */
    Engine::Nodes::Manager nodeManager;

    /** Current active scene root */
    std::shared_ptr<Engine::Nodes::Base> currentScene;

  private:
    friend class SceneLoader;

    /** Record an XML file used while loading the active scene */
    void recordSceneFile(const std::string &path);

    /** Scene name currently attached to the node manager */
    std::optional<std::string> activeScene;

    /** Scene name waiting to be applied at the next frame boundary */
    std::optional<std::string> queuedScene;

    /** Registered scene factories keyed by the names used with queueScene() */
    std::unordered_map<
        std::string,
        std::function<std::shared_ptr<Engine::Nodes::Base>()>>
        sceneFactories;

    /** Whether the game loop is currently running */
    bool running = false;

    /** Renderer clear colour */
    SDL_Color renderClearColour{0, 0, 0, 255};

    /** Adaptive frame limiter */
    Timer timer;

    /** Current scene XML files watched for development reloads */
    FileWatcher sceneFileWatcher;

    /** XML files used by the active scene and its imports */
    std::vector<std::string> sceneFiles;

    /** Persistent state save path */
    std::optional<std::filesystem::path> statePath;
};

} // namespace Engine
