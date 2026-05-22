#include "game.hpp"

#include "../../core/logger.hpp"
#include "../utils/format.hpp"
#include "resize/handler.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

namespace Engine {

namespace {

constexpr char SETTINGS_PATH[] = "resources/configuration/settings.yaml";
constexpr char STATE_DIRECTORY[] = ".saves";
constexpr char STATE_FILENAME[] = "state.yaml";

/** Return the development save path outside shipped resources */
std::filesystem::path
getStatePath(const std::filesystem::path &executableDirectory) {
    return executableDirectory / STATE_DIRECTORY / STATE_FILENAME;
}

} // namespace

Game::Game() : nodeManager(*this) {}

Game::~Game() {
    if(this->statePath.has_value() && !this->state.empty()) {
        try {
            this->saveState();
        } catch(const std::exception &exception) {
            spdlog::error("Failed to save game state: {}", exception.what());
        }
    }

    spdlog::info("Unloading game resources");
    this->resources.clear();

    Engine::Lifecycle::end(this->window, this->renderer);
}

void Game::start(
    const std::filesystem::path &executablePath,
    const std::string &consoleLogLevel
) {
    Logger::start(executablePath, consoleLogLevel);

    const std::filesystem::path executableDirectory =
        std::filesystem::weakly_canonical(
            std::filesystem::absolute(executablePath)
        )
            .parent_path();
    this->statePath = getStatePath(executableDirectory);

    // Bundled app launches use resources beside the executable
    if(std::filesystem::exists(executableDirectory / "resources")) {
        std::filesystem::current_path(executableDirectory);
    }

    if(std::filesystem::exists(*this->statePath)) {
        this->state.load(*this->statePath);
        spdlog::info(
            "Loaded game state from '{}'",
            Engine::Format::path(this->statePath->string())
        );
    } else {
        spdlog::debug(
            "No game state found at '{}'",
            Engine::Format::path(this->statePath->string())
        );
    }

    const Engine::Resource::ID settingsId =
        this->resources.load<Engine::Resource::YAML>(SETTINGS_PATH);

    const Engine::Resource::YAML &settingsResource =
        this->resources.get<Engine::Resource::YAML>(settingsId);
    const ::YAML::Node &settings = *settingsResource.node;

    this->timer.setTargetFrameRate(
        settings["renderer"]["target-frame-rate"].as<int>()
    );

    const std::string renderClearColour =
        settings["renderer"]["clear-colour"].as<std::string>();

    try {
        this->renderClearColour = Engine::Format::colour(renderClearColour);
    } catch(const std::exception &) {
        spdlog::warn(
            "Renderer clear colour '{}' is invalid; using fallback #000000ff",
            renderClearColour
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
    Uint32 previousFrameStartedAt = SDL_GetTicks();

    while(this->running) {
        const Uint32 frameStartedAt = SDL_GetTicks();

        // Scene switches happen at frame boundaries so hooks are paired
        // cleanly
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

        SDL_SetRenderDrawColor(
            this->renderer.get(),
            this->renderClearColour.r,
            this->renderClearColour.g,
            this->renderClearColour.b,
            this->renderClearColour.a
        );
        SDL_RenderClear(this->renderer.get());
        this->nodeManager.render(*this->renderer);
        SDL_RenderPresent(this->renderer.get());
        this->resources.purgeExpired();

        // Delay only after render and cache maintenance so frame work is
        // counted
        const Uint32 frameDuration = SDL_GetTicks() - frameStartedAt;
        this->timer.recordFrameDuration(frameDuration);

        const Uint32 delayDuration =
            this->timer.getDelayDuration(frameDuration);

        if(delayDuration > 0) {
            SDL_Delay(delayDuration);
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

void Game::saveState() const {
    if(!this->statePath.has_value()) {
        throw std::runtime_error("Game must be started before saving state");
    }

    std::filesystem::create_directories(this->statePath->parent_path());
    this->state.save(*this->statePath);
    spdlog::info(
        "Saved game state to '{}'",
        Engine::Format::path(this->statePath->string())
    );
}

SDL_Rect Game::getScreenSize() const {
    if(this->renderer == nullptr) {
        throw std::runtime_error(
            "Game must be started before querying screen size"
        );
    }

    SDL_Rect screenSize{0, 0, 0, 0};
    SDL_RenderGetLogicalSize(
        this->renderer.get(),
        &screenSize.w,
        &screenSize.h
    );

    if(screenSize.w <= 0 || screenSize.h <= 0) {
        SDL_GetRendererOutputSize(
            this->renderer.get(),
            &screenSize.w,
            &screenSize.h
        );
    }

    return screenSize;
}

void Game::queueScene(const std::string &name) {
    if(!this->sceneFactories.contains(name)) {
        throw std::runtime_error("Scene '" + name + "' is not registered");
    }

    this->queuedScene = name;
}

} // namespace Engine
