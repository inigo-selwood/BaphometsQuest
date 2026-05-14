#include "game.hpp"

#include "../parse/yaml.hpp"

namespace Engine {

namespace {

const char *CONFIG_PATH = "resources/configuration/game.yaml";
const char *CONFIG_CONTEXT = "Game config";

} // namespace

void Game::run() {
    spdlog::info("Entering game loop.");

    const AssetRegistry::AssetID configID =
        this->assets.loadYAML(CONFIG_PATH, CONFIG_CONTEXT);
    const YAML::Node &config = this->assets.get<YAML::Node>(configID);

    // Frame-rate configuration.
    const YAML::Node frameRate =
        Parse::YAML::requireMap(config, "frame-rate", CONFIG_CONTEXT);
    this->loopTimer.minimumFrameRate =
        Parse::YAML::requirePositiveValue<std::uint32_t>(
            frameRate,
            "minimum",
            CONFIG_CONTEXT
        );
    this->loopTimer.overrunHysteresisFrames =
        Parse::YAML::requirePositiveValue<int>(
            frameRate,
            "overrun-hysteresis-frames",
            CONFIG_CONTEXT
        );
    this->loopTimer.overrunToleranceMS =
        Parse::YAML::requirePositiveValue<std::uint32_t>(
            frameRate,
            "overrun-tolerance-ms",
            CONFIG_CONTEXT
        );
    this->loopTimer.step = Parse::YAML::requirePositiveValue<std::uint32_t>(
        frameRate,
        "step",
        CONFIG_CONTEXT
    );
    this->loopTimer.targetFrameRate =
        Parse::YAML::requirePositiveValue<std::uint32_t>(
            frameRate,
            "target",
            CONFIG_CONTEXT
        );
    this->loopTimer.underrunHeadroomMS =
        Parse::YAML::requirePositiveValue<std::uint32_t>(
            frameRate,
            "underrun-headroom-ms",
            CONFIG_CONTEXT
        );
    this->loopTimer.underrunHysteresisFrames =
        Parse::YAML::requirePositiveValue<int>(
            frameRate,
            "underrun-hysteresis-frames",
            CONFIG_CONTEXT
        );

    if(this->loopTimer.minimumFrameRate > this->loopTimer.targetFrameRate) {
        throw std::runtime_error(
            "Game config frame-rate.minimum must not exceed frame-rate.target."
        );
    }

    if(this->loopTimer.step > this->loopTimer.targetFrameRate) {
        throw std::runtime_error(
            "Game config frame-rate.step must not exceed frame-rate.target."
        );
    }

    this->assets.unload(configID);

    this->running = true;
    this->loopTimer.currentFrameRate = this->loopTimer.targetFrameRate;
    this->loopTimer.frameDelay = 1000 / this->loopTimer.currentFrameRate;
    this->loopTimer.overrunFrameCount = 0;
    this->loopTimer.underrunFrameCount = 0;
    this->loopTimer.frameStartTicks = SDL_GetTicks();

    spdlog::info("Targeting {} FPS.", this->loopTimer.currentFrameRate);

    SDL_Event event;

    while(this->running) {
        const std::uint32_t currentTicks = SDL_GetTicks();
        this->loopTimer.deltaTime =
            static_cast<float>(currentTicks - this->loopTimer.frameStartTicks)
            / 1000.0F;
        this->loopTimer.frameStartTicks = currentTicks;

        this->loadQueuedScene();
        this->input.beginFrame();

        while(SDL_PollEvent(&event) != 0) {
            this->input.handleEvent(event);

            if(event.type == SDL_QUIT) {
                spdlog::info("Received SDL quit event.");
                this->queueQuit();
                continue;
            }

            if(this->activeScene != nullptr) {
                this->activeScene->input(event);
            }
        }

        if(this->activeScene == nullptr) {
            this->queueQuit();
            continue;
        }

        this->activeScene->process(this->loopTimer.deltaTime);

        SDL_SetRenderDrawColor(
            this->SDLRenderer.get(),
            this->clearColour.r,
            this->clearColour.g,
            this->clearColour.b,
            this->clearColour.a
        );
        SDL_RenderClear(this->SDLRenderer.get());

        this->activeScene->render(this->SDLRenderer.get());
        SDL_RenderPresent(this->SDLRenderer.get());

        this->loopTimer.frameTicks =
            SDL_GetTicks() - this->loopTimer.frameStartTicks;
        this->adjustFrameRate(this->loopTimer.frameTicks);

        if(this->loopTimer.frameTicks < this->loopTimer.frameDelay) {
            SDL_Delay(this->loopTimer.frameDelay - this->loopTimer.frameTicks);
        }
    }

    this->destroyActiveScene();
    this->unloadActiveSceneAssets();

    spdlog::info("Exited game loop.");
}

void Game::adjustFrameRate(std::uint32_t workTicks) {
    if(workTicks
        > this->loopTimer.frameDelay + this->loopTimer.overrunToleranceMS) {
        this->loopTimer.overrunFrameCount++;
        this->loopTimer.underrunFrameCount = 0;

        if(this->loopTimer.overrunFrameCount
                >= this->loopTimer.overrunHysteresisFrames
            && this->loopTimer.currentFrameRate
                > this->loopTimer.minimumFrameRate) {
            this->loopTimer.currentFrameRate -= this->loopTimer.step;
            this->loopTimer.frameDelay =
                1000 / this->loopTimer.currentFrameRate;
            this->loopTimer.overrunFrameCount = 0;

            spdlog::warn(
                "Reducing frame rate to {} FPS; frame work took {}ms.",
                this->loopTimer.currentFrameRate,
                workTicks
            );
        }

        return;
    }

    if(workTicks + this->loopTimer.underrunHeadroomMS
            < this->loopTimer.frameDelay
        && this->loopTimer.currentFrameRate
            < this->loopTimer.targetFrameRate) {
        this->loopTimer.underrunFrameCount++;
        this->loopTimer.overrunFrameCount = 0;

        if(this->loopTimer.underrunFrameCount
            >= this->loopTimer.underrunHysteresisFrames) {
            this->loopTimer.currentFrameRate += this->loopTimer.step;
            this->loopTimer.frameDelay =
                1000 / this->loopTimer.currentFrameRate;
            this->loopTimer.underrunFrameCount = 0;

            spdlog::info(
                "Increasing frame rate to {} FPS; frame work took {}ms.",
                this->loopTimer.currentFrameRate,
                workTicks
            );
        }

        return;
    }

    this->loopTimer.overrunFrameCount = 0;
    this->loopTimer.underrunFrameCount = 0;
}

} // namespace Engine
