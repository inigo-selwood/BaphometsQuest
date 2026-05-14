#include "game.hpp"

#include "../parse/parse.hpp"
#include "../parse/yaml.hpp"

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

namespace Engine {

namespace {

const char *CONFIG_PATH = "resources/configuration/game.yaml";
const char *CONFIG_CONTEXT = "Game config";

} // namespace

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

    const AssetRegistry::AssetID configID =
        this->assets.loadYAML(CONFIG_PATH, CONFIG_CONTEXT);
    const YAML::Node &config = this->assets.get<YAML::Node>(configID);

    // Audio subsystem.
    const YAML::Node audio =
        Parse::YAML::requireMap(config, "audio", CONFIG_CONTEXT);
    const int audioChannels = Parse::YAML::requirePositiveValue<int>(
        audio,
        "channels",
        CONFIG_CONTEXT
    );
    const int audioChunkSize = Parse::YAML::requirePositiveValue<int>(
        audio,
        "chunk-size",
        CONFIG_CONTEXT
    );
    const int audioFrequency = Parse::YAML::requirePositiveValue<int>(
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
    const YAML::Node render =
        Parse::YAML::requireMap(config, "render", CONFIG_CONTEXT);
    const std::string scaleQuality =
        Parse::YAML::requireString(render, "scale-quality", CONFIG_CONTEXT);
    this->clearColour = Parse::colour(
        Parse::YAML::requireString(render, "clear-colour", CONFIG_CONTEXT),
        "render.clear-colour"
    );

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality.c_str());

    // Window creation.
    const YAML::Node window =
        Parse::YAML::requireMap(config, "window", CONFIG_CONTEXT);
    const int logicalHeight = Parse::YAML::requirePositiveValue<int>(
        window,
        "logical-height",
        CONFIG_CONTEXT
    );
    const int logicalWidth = Parse::YAML::requirePositiveValue<int>(
        window,
        "logical-width",
        CONFIG_CONTEXT
    );
    const int windowScale = Parse::YAML::requirePositiveValue<int>(
        window,
        "scale",
        CONFIG_CONTEXT
    );
    const bool windowResizable =
        Parse::YAML::requireBool(window, "resizable", CONFIG_CONTEXT);
    const std::string windowTitle =
        Parse::YAML::requireString(window, "title", CONFIG_CONTEXT);
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

    this->assets.unload(configID);
}

void Game::quit() {
    spdlog::info("Shutting down game systems.");

    this->running = false;

    this->destroyActiveScene();
    this->unloadActiveSceneAssets();

    this->queuedSceneName.reset();

    this->SDLRenderer.reset();
    this->SDLWindow.reset();
    spdlog::debug("Destroyed SDL renderer and window.");

    this->assets.unloadAll();

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

} // namespace Engine
