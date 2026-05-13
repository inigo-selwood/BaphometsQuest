#include "game.hpp"

#include "../properties/property.hpp"
#include "../validation/validation.hpp"

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

namespace Engine {

namespace {

const char *CONFIG_PATH = "resources/configuration/game.yaml";
const char *CONFIG_CONTEXT = "Game config";

} // namespace

Game &Game::getInstance() {
    static Game game;
    return game;
}

AssetRegistry &Game::getAssets() {
    return this->assets;
}

void Game::queueQuit() {
    spdlog::info("Queueing game quit.");
    this->running = false;
}

void Game::queueScene(const std::string &name) {
    if(!this->sceneFactories.contains(name)) {
        throw std::runtime_error("Unknown scene: " + name);
    }

    spdlog::info("Queueing scene '{}'.", name);
    this->queuedSceneName = name;
}

void Game::start() {
    spdlog::info("Starting game systems.");

    // Core SDL systems.
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        throw std::runtime_error(
            std::string("Failed to initialize SDL: ") + SDL_GetError()
        );
    }

    this->SDLStarted = true;
    spdlog::debug("Initialized SDL.");

    // Image subsystem.
    if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        throw std::runtime_error(
            std::string("Failed to initialize SDL_image: ") + IMG_GetError()
        );
    }

    this->SDLImageStarted = true;
    spdlog::debug("Initialized SDL_image with PNG support.");

    // Audio subsystem.
    const YAML::Node audio = Validation::requireMap(
        this->assets.getYAML(CONFIG_PATH, CONFIG_CONTEXT),
        "audio",
        CONFIG_CONTEXT
    );
    const int audioChannels = Validation::requirePositiveValue<int>(
        audio,
        "channels",
        CONFIG_CONTEXT
    );
    const int audioChunkSize = Validation::requirePositiveValue<int>(
        audio,
        "chunk-size",
        CONFIG_CONTEXT
    );
    const int audioFrequency = Validation::requirePositiveValue<int>(
        audio,
        "frequency",
        CONFIG_CONTEXT
    );

    if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        throw std::runtime_error(
            std::string("Failed to initialize SDL_mixer: ") + Mix_GetError()
        );
    }

    this->SDLMixerStarted = true;
    spdlog::debug("Initialized SDL_mixer with OGG support.");

    if(Mix_OpenAudio(
           audioFrequency,
           AUDIO_S16SYS,
           audioChannels,
           audioChunkSize
       ) != 0) {
        throw std::runtime_error(
            std::string("Failed to open SDL_mixer audio: ") + Mix_GetError()
        );
    }

    this->SDLAudioOpened = true;
    spdlog::debug(
        "Opened audio at {}Hz with {} channel(s) and {} byte chunks.",
        audioFrequency,
        audioChannels,
        audioChunkSize
    );

    // Font subsystem.
    if(TTF_Init() != 0) {
        throw std::runtime_error(
            std::string("Failed to initialize SDL_ttf: ") + TTF_GetError()
        );
    }

    this->SDLTTFStarted = true;
    spdlog::debug("Initialized SDL_ttf.");

    // Render configuration.
    const YAML::Node render = Validation::requireMap(
        this->assets.getYAML(CONFIG_PATH, CONFIG_CONTEXT),
        "render",
        CONFIG_CONTEXT
    );
    const std::string scaleQuality =
        Validation::requireString(render, "scale-quality", CONFIG_CONTEXT);
    this->clearColour = Property::parseColour(
        Validation::requireString(render, "clear-colour", CONFIG_CONTEXT),
        "render.clear-colour"
    );

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality.c_str());

    // Window creation.
    const YAML::Node window = Validation::requireMap(
        this->assets.getYAML(CONFIG_PATH, CONFIG_CONTEXT),
        "window",
        CONFIG_CONTEXT
    );
    const int logicalHeight = Validation::requirePositiveValue<int>(
        window,
        "logical-height",
        CONFIG_CONTEXT
    );
    const int logicalWidth = Validation::requirePositiveValue<int>(
        window,
        "logical-width",
        CONFIG_CONTEXT
    );
    const int windowScale =
        Validation::requirePositiveValue<int>(window, "scale", CONFIG_CONTEXT);
    const bool windowResizable =
        Validation::requireBool(window, "resizable", CONFIG_CONTEXT);
    const std::string windowTitle =
        Validation::requireString(window, "title", CONFIG_CONTEXT);
    std::uint32_t windowFlags = SDL_WINDOW_SHOWN;

    if(windowResizable) {
        windowFlags |= SDL_WINDOW_RESIZABLE;
    }

    this->SDLWindow.reset(SDL_CreateWindow(
        windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        logicalWidth * windowScale,
        logicalHeight * windowScale,
        windowFlags
    ));

    if(this->SDLWindow == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create window: ") + SDL_GetError()
        );
    }

    spdlog::info(
        "Created window {}x{}.",
        logicalWidth * windowScale,
        logicalHeight * windowScale
    );

    // Renderer creation.
    this->SDLRenderer.reset(
        SDL_CreateRenderer(this->SDLWindow.get(), -1, SDL_RENDERER_ACCELERATED)
    );

    if(this->SDLRenderer == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create renderer: ") + SDL_GetError()
        );
    }

    if(SDL_RenderSetLogicalSize(
           this->SDLRenderer.get(),
           logicalWidth,
           logicalHeight
       ) != 0) {
        throw std::runtime_error(
            std::string("Failed to set logical render size: ") + SDL_GetError()
        );
    }

    spdlog::info("Created accelerated SDL renderer.");
    spdlog::info(
        "Rendering at {}x{} scaled {}x to {}x{}.",
        logicalWidth,
        logicalHeight,
        windowScale,
        logicalWidth * windowScale,
        logicalHeight * windowScale
    );
}

void Game::run() {
    spdlog::info("Entering game loop.");

    // Frame-rate configuration.
    const YAML::Node frameRate = Validation::requireMap(
        this->assets.getYAML(CONFIG_PATH, CONFIG_CONTEXT),
        "frame-rate",
        CONFIG_CONTEXT
    );
    this->loopTimer.minimumFrameRate =
        Validation::requirePositiveValue<std::uint32_t>(
            frameRate,
            "minimum",
            CONFIG_CONTEXT
        );
    this->loopTimer.overrunHysteresisFrames =
        Validation::requirePositiveValue<int>(
            frameRate,
            "overrun-hysteresis-frames",
            CONFIG_CONTEXT
        );
    this->loopTimer.overrunToleranceMS =
        Validation::requirePositiveValue<std::uint32_t>(
            frameRate,
            "overrun-tolerance-ms",
            CONFIG_CONTEXT
        );
    this->loopTimer.step = Validation::requirePositiveValue<std::uint32_t>(
        frameRate,
        "step",
        CONFIG_CONTEXT
    );
    this->loopTimer.targetFrameRate =
        Validation::requirePositiveValue<std::uint32_t>(
            frameRate,
            "target",
            CONFIG_CONTEXT
        );
    this->loopTimer.underrunHeadroomMS =
        Validation::requirePositiveValue<std::uint32_t>(
            frameRate,
            "underrun-headroom-ms",
            CONFIG_CONTEXT
        );
    this->loopTimer.underrunHysteresisFrames =
        Validation::requirePositiveValue<int>(
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

        while(SDL_PollEvent(&event) != 0) {
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

    if(this->activeScene != nullptr) {
        this->activeScene->exitTree();
    }

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

void Game::quit() {
    spdlog::info("Shutting down game systems.");

    this->running = false;

    if(this->activeScene != nullptr) {
        spdlog::debug("Destroying active scene.");
        this->activeScene->exitTree();
        this->activeScene.reset();
    }

    this->queuedSceneName.reset();

    this->SDLRenderer.reset();
    this->SDLWindow.reset();
    spdlog::debug("Destroyed SDL renderer and window.");

    this->assets.clear();

    if(this->SDLAudioOpened) {
        Mix_CloseAudio();
        this->SDLAudioOpened = false;
        spdlog::debug("Closed SDL_mixer audio.");
    }

    if(this->SDLTTFStarted) {
        TTF_Quit();
        this->SDLTTFStarted = false;
        spdlog::debug("Shut down SDL_ttf.");
    }

    if(this->SDLMixerStarted) {
        Mix_Quit();
        this->SDLMixerStarted = false;
        spdlog::debug("Shut down SDL_mixer.");
    }

    if(this->SDLImageStarted) {
        IMG_Quit();
        this->SDLImageStarted = false;
        spdlog::debug("Shut down SDL_image.");
    }

    if(this->SDLStarted) {
        SDL_Quit();
        this->SDLStarted = false;
        spdlog::debug("Shut down SDL.");
    }

    spdlog::info("Game systems shut down.");
}

void Game::loadQueuedScene() {
    if(!this->queuedSceneName.has_value()) {
        return;
    }

    if(this->activeScene != nullptr) {
        spdlog::debug("Exiting active scene.");
        this->activeScene->exitTree();
    }

    spdlog::info("Loading scene '{}'.", *this->queuedSceneName);
    this->activeScene = this->sceneFactories.at(*this->queuedSceneName)();
    this->activeScene->enterTree();
    spdlog::info("Scene '{}' loaded.", *this->queuedSceneName);
    this->queuedSceneName.reset();
}

} // namespace Engine
