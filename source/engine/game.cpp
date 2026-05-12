#include "game.hpp"

namespace {

const std::uint32_t FRAME_RATE_STEP = 1;
const std::uint32_t FRAME_RATE = 24;
const std::uint32_t MIN_FRAME_RATE = 12;

const std::uint32_t FRAME_UNDERRUN_HEADROOM = 4;
const std::uint32_t FRAME_OVERRUN_TOLERANCE = 2;
const int OVERRUN_HYSTERESIS_FRAMES = 12;
const int UNDERRUN_HYSTERESIS_FRAMES = 90;

const int LOGICAL_HEIGHT = 192;
const int LOGICAL_WIDTH = 256;

const int WINDOW_SCALE = 3;
const int WINDOW_HEIGHT = LOGICAL_HEIGHT * WINDOW_SCALE;
const int WINDOW_WIDTH = LOGICAL_WIDTH * WINDOW_SCALE;

} // namespace

Game &Game::getInstance() {
    static Game game;
    return game;
}

AssetRegistry &Game::getAssets() {
    return assets;
}

void Game::queueQuit() {
    spdlog::info("Queueing game quit.");
    running = false;
}

void Game::queueScene(const std::string &name) {
    if (!sceneFactories.contains(name)) {
        throw std::runtime_error("Unknown scene: " + name);
    }

    spdlog::info("Queueing scene '{}'.", name);
    queuedSceneName = name;
}

void Game::start() {
    spdlog::info("Starting game systems.");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        throw std::runtime_error(
                std::string("Failed to initialize SDL: ") + SDL_GetError());
    }

    SDLStarted = true;
    spdlog::debug("Initialized SDL.");

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        throw std::runtime_error(
                std::string("Failed to initialize SDL_image: ")
                + IMG_GetError());
    }

    SDLImageStarted = true;
    spdlog::debug("Initialized SDL_image with PNG support.");

    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        throw std::runtime_error(
                std::string("Failed to initialize SDL_mixer: ")
                + Mix_GetError());
    }

    SDLMixerStarted = true;
    spdlog::debug("Initialized SDL_mixer with OGG support.");

    if (TTF_Init() != 0) {
        throw std::runtime_error(std::string("Failed to initialize SDL_ttf: ")
                + TTF_GetError());
    }

    SDLTTFStarted = true;
    spdlog::debug("Initialized SDL_ttf.");

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    SDLWindow.reset(SDL_CreateWindow("Untitled Game",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN));

    if (SDLWindow == nullptr) {
        throw std::runtime_error(
                std::string("Failed to create window: ") + SDL_GetError());
    }

    spdlog::info("Created window {}x{}.", WINDOW_WIDTH, WINDOW_HEIGHT);

    SDLRenderer.reset(
            SDL_CreateRenderer(SDLWindow.get(), -1, SDL_RENDERER_ACCELERATED));

    if (SDLRenderer == nullptr) {
        throw std::runtime_error(
                std::string("Failed to create renderer: ") + SDL_GetError());
    }

    if (SDL_RenderSetLogicalSize(
                SDLRenderer.get(), LOGICAL_WIDTH, LOGICAL_HEIGHT)
            != 0) {
        throw std::runtime_error(
                std::string("Failed to set logical render size: ")
                + SDL_GetError());
    }

    spdlog::info("Created accelerated SDL renderer.");
    spdlog::info("Rendering at {}x{} scaled {}x to {}x{}.",
            LOGICAL_WIDTH,
            LOGICAL_HEIGHT,
            WINDOW_SCALE,
            WINDOW_WIDTH,
            WINDOW_HEIGHT);
}

void Game::run() {
    spdlog::info("Entering game loop.");

    running = true;
    currentFrameRate = FRAME_RATE;
    frameDelay = 1000 / currentFrameRate;
    overrunFrameCount = 0;
    underrunFrameCount = 0;
    frameStartTicks = SDL_GetTicks();

    spdlog::info("Targeting {} FPS.", currentFrameRate);

    SDL_Event event;

    while (running) {
        const std::uint32_t currentTicks = SDL_GetTicks();
        deltaTime =
                static_cast<float>(currentTicks - frameStartTicks) / 1000.0F;
        frameStartTicks = currentTicks;

        loadQueuedScene();

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                spdlog::info("Received SDL quit event.");
                queueQuit();
                continue;
            }

            if (activeScene != nullptr) {
                activeScene->input(event);
            }
        }

        if (activeScene == nullptr) {
            queueQuit();
            continue;
        }

        activeScene->update(deltaTime);

        SDL_SetRenderDrawColor(SDLRenderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(SDLRenderer.get());

        activeScene->render(SDLRenderer.get());
        SDL_RenderPresent(SDLRenderer.get());

        frameTicks = SDL_GetTicks() - frameStartTicks;
        adjustFrameRate(frameTicks);

        if (frameTicks < frameDelay) {
            SDL_Delay(frameDelay - frameTicks);
        }
    }

    if (activeScene != nullptr) {
        activeScene->exitTree();
    }

    spdlog::info("Exited game loop.");
}

void Game::adjustFrameRate(std::uint32_t workTicks) {
    if (workTicks > frameDelay + FRAME_OVERRUN_TOLERANCE) {
        overrunFrameCount++;
        underrunFrameCount = 0;

        if (overrunFrameCount >= OVERRUN_HYSTERESIS_FRAMES
                && currentFrameRate > MIN_FRAME_RATE) {
            currentFrameRate -= FRAME_RATE_STEP;
            frameDelay = 1000 / currentFrameRate;
            overrunFrameCount = 0;

            spdlog::warn(
                    "Reducing frame rate to {} FPS; frame work took {}ms.",
                    currentFrameRate,
                    workTicks);
        }

        return;
    }

    if (workTicks + FRAME_UNDERRUN_HEADROOM < frameDelay
            && currentFrameRate < FRAME_RATE) {
        underrunFrameCount++;
        overrunFrameCount = 0;

        if (underrunFrameCount >= UNDERRUN_HYSTERESIS_FRAMES) {
            currentFrameRate += FRAME_RATE_STEP;
            frameDelay = 1000 / currentFrameRate;
            underrunFrameCount = 0;

            spdlog::info(
                    "Increasing frame rate to {} FPS; frame work took {}ms.",
                    currentFrameRate,
                    workTicks);
        }

        return;
    }

    overrunFrameCount = 0;
    underrunFrameCount = 0;
}

void Game::quit() {
    spdlog::info("Shutting down game systems.");

    running = false;

    if (activeScene != nullptr) {
        spdlog::debug("Destroying active scene.");
        activeScene->exitTree();
        activeScene.reset();
    }

    queuedSceneName.reset();

    SDLRenderer.reset();
    SDLWindow.reset();
    spdlog::debug("Destroyed SDL renderer and window.");

    assets.clear();

    if (SDLTTFStarted) {
        TTF_Quit();
        SDLTTFStarted = false;
        spdlog::debug("Shut down SDL_ttf.");
    }

    if (SDLMixerStarted) {
        Mix_Quit();
        SDLMixerStarted = false;
        spdlog::debug("Shut down SDL_mixer.");
    }

    if (SDLImageStarted) {
        IMG_Quit();
        SDLImageStarted = false;
        spdlog::debug("Shut down SDL_image.");
    }

    if (SDLStarted) {
        SDL_Quit();
        SDLStarted = false;
        spdlog::debug("Shut down SDL.");
    }

    spdlog::info("Game systems shut down.");
}

void Game::loadQueuedScene() {
    if (!queuedSceneName.has_value()) {
        return;
    }

    if (activeScene != nullptr) {
        spdlog::debug("Exiting active scene.");
        activeScene->exitTree();
    }

    spdlog::info("Loading scene '{}'.", *queuedSceneName);
    activeScene = sceneFactories.at(*queuedSceneName)();
    activeScene->enterTree();
    spdlog::info("Scene '{}' loaded.", *queuedSceneName);
    queuedSceneName.reset();
}
