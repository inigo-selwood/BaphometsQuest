#include "game.hpp"

#include "../../logger.hpp"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

namespace Engine {

namespace {

constexpr char SETTINGS_PATH[] = "resources/configuration/settings.yaml";

} // namespace

Game::~Game() {
    Engine::Lifecycle::end(this->window, this->renderer);

    spdlog::info("Unloading game resources");
    this->resources.clear();
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
        throw std::runtime_error("Target frame rate must be greater than zero");
    }

    spdlog::info("Starting game services");

    Engine::Lifecycle::start(
        this->window,
        this->renderer,
        settingsResource
    );
}

void Game::run() {
    if(this->renderer == nullptr) {
        throw std::runtime_error("Game must be started before it can run");
    }

    spdlog::info("Starting game run");

    this->running = true;

    SDL_Event event;
    const Uint32 targetFrameDuration =
        static_cast<Uint32>(1000 / this->frameRate);

    while(this->running) {
        const Uint32 frameStartedAt = SDL_GetTicks();

        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                this->running = false;
            }
        }

        SDL_SetRenderDrawColor(this->renderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(this->renderer.get());
        SDL_RenderPresent(this->renderer.get());

        const Uint32 frameDuration = SDL_GetTicks() - frameStartedAt;

        if(frameDuration < targetFrameDuration) {
            SDL_Delay(targetFrameDuration - frameDuration);
        }
    }

    spdlog::info("Finished game run");
}

void Game::queueQuit() {
    this->running = false;
}

} // namespace Engine
