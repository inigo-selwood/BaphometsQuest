#pragma once

#include "assetRegistry.hpp"
#include "includes.hpp"
#include "scene.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

class Game {
  public:
    static Game &getInstance();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;

    template <typename SceneType> void registerScene(const std::string &name) {
        static_assert(std::is_base_of_v<Scene, SceneType>,
                "SceneType must inherit from Scene.");

        spdlog::debug("Registering scene '{}'.", name);

        sceneFactories[name] = [] { return std::make_unique<SceneType>(); };
    }

    AssetRegistry &getAssets();

    void queueQuit();
    void queueScene(const std::string &name);
    void quit();
    void run();
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

    Game() = default;

    void adjustFrameRate(std::uint32_t workTicks);
    void loadQueuedScene();

    std::unique_ptr<SDL_Window, WindowDeleter> SDLWindow;
    std::unique_ptr<SDL_Renderer, RendererDeleter> SDLRenderer;

    bool running = false;
    bool SDLStarted = false;
    bool SDLImageStarted = false;
    bool SDLMixerStarted = false;
    bool SDLTTFStarted = false;
    std::uint32_t frameStartTicks = 0;
    std::uint32_t frameTicks = 0;
    std::uint32_t frameDelay = 0;
    std::uint32_t currentFrameRate = 0;
    float deltaTime = 0.0F;
    int overrunFrameCount = 0;
    int underrunFrameCount = 0;

    AssetRegistry assets;
    std::unique_ptr<Scene> activeScene;
    std::optional<std::string> queuedSceneName;
    std::unordered_map<std::string, std::function<std::unique_ptr<Scene>()>>
            sceneFactories;
};
