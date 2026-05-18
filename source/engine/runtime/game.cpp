#include "game.hpp"

#include "../../core/logger.hpp"
#include "resize/handler.hpp"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

namespace Engine {

namespace {

constexpr char SETTINGS_PATH[] = "resources/configuration/settings.yaml";

} // namespace

Game::Game() : nodeManager(*this) {}

Game::~Game() {
    spdlog::info("Unloading game resources");
    this->resources.clear();

    Engine::Lifecycle::end(this->window, this->renderer);
}

void Game::start(
    const std::filesystem::path &executablePath,
    const std::string &consoleLogLevel
) {
    Logger::start(executablePath, consoleLogLevel);

    const Engine::Resource::ID settingsId =
        this->resources.load<Engine::Resource::YAML>(SETTINGS_PATH);

    const Engine::Resource::YAML &settingsResource =
        this->resources.get<Engine::Resource::YAML>(settingsId);
    const ::YAML::Node &settings = *settingsResource.node;

    this->frameRate = settings["game"]["target-frame-rate"].as<int>();

    if(this->frameRate <= 0) {
        throw std::runtime_error(
            "Target frame rate must be greater than zero"
        );
    }

    spdlog::info("Starting game services");

    Engine::Lifecycle::start(this->window, this->renderer, settingsResource);
}

void Game::run() {
    if(this->renderer == nullptr) {
        throw std::runtime_error("Game must be started before it can run");
    }

    if(this->currentScene == nullptr && !this->queuedScene.has_value()) {
        throw std::runtime_error("Game must have a scene before it can run");
    }

    spdlog::info("Starting game run");

    this->running = true;
    bool sceneEntered = false;

    SDL_Event event;
    const Uint32 targetFrameDuration =
        static_cast<Uint32>(1000 / this->frameRate);
    Uint32 previousFrameStartedAt = SDL_GetTicks();

    while(this->running) {
        const Uint32 frameStartedAt = SDL_GetTicks();

        if(this->queuedScene.has_value()) {
            const std::string sceneName = *this->queuedScene;

            if(sceneEntered) {
                this->nodeManager.exit();
            }

            this->currentScene = this->sceneFactories.at(sceneName)();
            this->queuedScene.reset();
            this->nodeManager.setRoot(this->currentScene);
            spdlog::debug("Entering scene '{}'", sceneName);
            this->nodeManager.enter();
            sceneEntered = true;
        } else if(!sceneEntered) {
            this->nodeManager.setRoot(this->currentScene);
            spdlog::debug("Entering current scene");
            this->nodeManager.enter();
            sceneEntered = true;
        }

        const float deltaSeconds =
            static_cast<float>(frameStartedAt - previousFrameStartedAt)
            / 1000.0F;
        previousFrameStartedAt = frameStartedAt;

        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                this->running = false;
            } else if(event.type == SDL_WINDOWEVENT) {
                Engine::ResizeHandler::handle(
                    this->window.get(),
                    this->renderer.get(),
                    event
                );
                this->nodeManager.input(event);
            } else {
                this->nodeManager.input(event);
            }
        }

        this->nodeManager.process(deltaSeconds);

        SDL_SetRenderDrawColor(this->renderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(this->renderer.get());
        this->nodeManager.render(*this->renderer);
        SDL_RenderPresent(this->renderer.get());
        this->resources.purgeExpired();

        const Uint32 frameDuration = SDL_GetTicks() - frameStartedAt;

        if(frameDuration < targetFrameDuration) {
            SDL_Delay(targetFrameDuration - frameDuration);
        }
    }

    if(sceneEntered) {
        this->nodeManager.exit();
    }

    spdlog::info("Finished game run");
}

void Game::queueQuit() {
    this->running = false;
}

void Game::queueScene(const std::string &name) {
    if(!this->sceneFactories.contains(name)) {
        throw std::runtime_error("Scene '" + name + "' is not registered");
    }

    this->queuedScene = name;
}

} // namespace Engine
