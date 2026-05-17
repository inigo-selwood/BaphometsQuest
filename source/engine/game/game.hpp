#pragma once

#include "../assets/assetRegistry.hpp"
#include "../node/node.hpp"

#include "inputState.hpp"

#include <SDL.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <spdlog/spdlog.h>

namespace Engine {

/**
 * Owns SDL systems, the active scene, assets, and the main loop.
 *
 * Game is a process-wide singleton because nodes need access to shared engine
 * services such as the asset registry and quit queueing.
 */
class Game {
  public:
    /** Return the single game instance. */
    static Game &getInstance();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;

    /**
     * Register a scene type that can later be queued by name.
     *
     * @tparam SceneType Concrete node type used as a scene root.
     */
    template <typename SceneType> void registerScene(const std::string &name) {
        static_assert(
            std::is_base_of_v<Node, SceneType>,
            "SceneType must inherit from Node."
        );

        spdlog::debug("Registering scene '{}'.", name);

        this->sceneFactories[name] = [] {
            return std::make_unique<SceneType>();
        };
    }

    /** Return the shared asset registry. */
    AssetRegistry &getAssets();

    /** Return the current input snapshot. */
    const InputState &getInput() const;

    /** Request the game loop to exit at the next loop boundary. */
    void queueQuit();

    /**
     * Queue a registered scene to become active.
     *
     * @throws std::runtime_error if the scene name is unknown.
     */
    void queueScene(const std::string &name);

    /** Shut down SDL systems and release active resources. */
    void quit();

    /** Run the main loop until quit is queued. */
    void run();

    /** Initialize SDL systems and create the window/renderer. */
    void start();

  private:
    struct WindowDeleter {
        void operator()(SDL_Window *window) const {
            SDL_DestroyWindow(window);
        }
    };

    struct RendererDeleter {
        void operator()(SDL_Renderer *renderer) const {
            SDL_DestroyRenderer(renderer);
        }
    };

    struct LoopTimer {
        std::uint32_t currentFrameRate = 0;
        std::uint32_t frameDelay = 0;
        std::uint32_t minimumFrameRate = 0;
        std::uint32_t step = 0;
        std::uint32_t targetFrameRate = 0;

        float deltaTime = 0.0F;
        std::uint32_t frameStartTicks = 0;
        std::uint32_t frameTicks = 0;

        int overrunFrameCount = 0;
        int overrunHysteresisFrames = 0;
        std::uint32_t overrunToleranceMS = 0;

        int underrunFrameCount = 0;
        int underrunHysteresisFrames = 0;
        std::uint32_t underrunHeadroomMS = 0;
    };

    Game() = default;

    void adjustFrameRate(std::uint32_t workTicks);
    void destroyActiveScene();
    void loadQueuedScene();
    void unloadActiveSceneAssets();

    std::unique_ptr<SDL_Window, WindowDeleter> SDLWindow;
    std::unique_ptr<SDL_Renderer, RendererDeleter> SDLRenderer;

    SDL_Color clearColour{0, 0, 0, 255};

    bool running = false;
    bool SDLStarted = false;
    bool SDLAudioOpened = false;
    bool SDLImageStarted = false;
    bool SDLMixerStarted = false;
    bool SDLTTFStarted = false;

    AssetRegistry assets;
    std::unique_ptr<Node> activeScene;
    InputState input;
    LoopTimer loopTimer;
    std::optional<std::string> queuedSceneName;
    std::unordered_map<std::string, std::function<std::unique_ptr<Node>()>>
        sceneFactories;
};

} // namespace Engine
