#pragma once

#include "../resources/types/yaml.hpp"
#include "resize/handler.hpp"

#include <memory>
#include <stdexcept>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <spdlog/spdlog.h>

namespace Engine::Lifecycle {

inline constexpr int DEFAULT_AUDIO_CHANNELS = 2;
inline constexpr int DEFAULT_AUDIO_CHUNK_SIZE = 2048;
inline constexpr int DEFAULT_AUDIO_FREQUENCY = 44100;
inline constexpr int MIN_WINDOW_SIZE = 1;
inline constexpr float DEFAULT_RENDERER_SCALE = 1.0F;

/** Start SDL subsystems and create the active window and renderer */
template <typename WindowPointer, typename RendererPointer>
void start(
    WindowPointer &window,
    RendererPointer &renderer,
    const Engine::Resource::YAML &settings
) {
    try {
        spdlog::info("Starting SDL lifecycle");

        // Core SDL subsystems
        if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error(
                std::string("Failed to initialize SDL: ") + SDL_GetError()
            );
        }

        spdlog::debug("Initialized SDL");

        if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
            throw std::runtime_error(
                std::string("Failed to initialize SDL_image: ")
                + IMG_GetError()
            );
        }

        spdlog::debug("Initialized SDL_image with PNG support");

        // Audio subsystem and settings
        if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
            throw std::runtime_error(
                std::string("Failed to initialize SDL_mixer: ")
                + Mix_GetError()
            );
        }

        spdlog::debug("Initialized SDL_mixer with OGG support");

        int audioFrequency = (*settings.node)["audio"]["frequency"].as<int>();
        int audioChannels = (*settings.node)["audio"]["channels"].as<int>();
        int audioChunkSize = (*settings.node)["audio"]["chunk-size"].as<int>();

        if(audioFrequency <= 0) {
            spdlog::warn(
                "Audio frequency {} is invalid; using {}",
                audioFrequency,
                DEFAULT_AUDIO_FREQUENCY
            );
            audioFrequency = DEFAULT_AUDIO_FREQUENCY;
        }

        if(audioChannels <= 0) {
            spdlog::warn(
                "Audio channels {} is invalid; using {}",
                audioChannels,
                DEFAULT_AUDIO_CHANNELS
            );
            audioChannels = DEFAULT_AUDIO_CHANNELS;
        }

        if(audioChunkSize <= 0) {
            spdlog::warn(
                "Audio chunk size {} is invalid; using {}",
                audioChunkSize,
                DEFAULT_AUDIO_CHUNK_SIZE
            );
            audioChunkSize = DEFAULT_AUDIO_CHUNK_SIZE;
        }

        if(Mix_OpenAudio(
               audioFrequency,
               MIX_DEFAULT_FORMAT,
               audioChannels,
               audioChunkSize
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to open SDL_mixer audio: ")
                + Mix_GetError()
            );
        }

        spdlog::debug("Opened SDL_mixer audio");

        // Font subsystem
        if(TTF_Init() != 0) {
            throw std::runtime_error(
                std::string("Failed to initialize SDL_ttf: ") + TTF_GetError()
            );
        }

        spdlog::debug("Initialized SDL_ttf");

        // Window and renderer settings
        const std::string windowTitle =
            (*settings.node)["window"]["title"].as<std::string>();
        int windowWidth = (*settings.node)["window"]["width"].as<int>();
        int windowHeight = (*settings.node)["window"]["height"].as<int>();
        float rendererScale =
            (*settings.node)["renderer"]["scale"].as<float>();

        if(windowWidth < MIN_WINDOW_SIZE) {
            spdlog::warn(
                "Window width {} is below minimum; using {}",
                windowWidth,
                MIN_WINDOW_SIZE
            );
            windowWidth = MIN_WINDOW_SIZE;
        }

        if(windowHeight < MIN_WINDOW_SIZE) {
            spdlog::warn(
                "Window height {} is below minimum; using {}",
                windowHeight,
                MIN_WINDOW_SIZE
            );
            windowHeight = MIN_WINDOW_SIZE;
        }

        if(rendererScale <= 0) {
            spdlog::warn(
                "Renderer scale {} is invalid; using {}",
                rendererScale,
                DEFAULT_RENDERER_SCALE
            );
            rendererScale = DEFAULT_RENDERER_SCALE;
        }

        // Window creation
        window.reset(SDL_CreateWindow(
            windowTitle.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(static_cast<float>(windowWidth) * rendererScale),
            static_cast<int>(static_cast<float>(windowHeight) * rendererScale),
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        ));

        if(window == nullptr) {
            throw std::runtime_error(
                std::string("Failed to create window: ") + SDL_GetError()
            );
        }

        // Window icon
        constexpr char WINDOW_ICON_PATH[] = "resources/textures/Favicon.png";
        std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> windowIcon(
            IMG_Load(WINDOW_ICON_PATH),
            SDL_FreeSurface
        );

        if(windowIcon == nullptr) {
            throw std::runtime_error(
                std::string("Failed to load window icon: ") + IMG_GetError()
            );
        }

        SDL_SetWindowIcon(window.get(), windowIcon.get());
        spdlog::debug("Set window icon '{}'", WINDOW_ICON_PATH);

        // Renderer creation
        renderer.reset(
            SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED)
        );

        if(renderer == nullptr) {
            spdlog::debug(
                "Falling back to software renderer: {}",
                SDL_GetError()
            );

            renderer.reset(
                SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_SOFTWARE)
            );
        }

        if(renderer == nullptr) {
            throw std::runtime_error(
                std::string("Failed to create renderer: ") + SDL_GetError()
            );
        }

        // Logical rendering area and resize behaviour
        if(SDL_RenderSetLogicalSize(renderer.get(), windowWidth, windowHeight)
            != 0) {
            throw std::runtime_error(
                std::string("Failed to set renderer logical size: ")
                + SDL_GetError()
            );
        }

        Engine::ResizeHandler::lockAspectRatio(window.get(), renderer.get());
    } catch(...) {
        // Roll back partially started SDL state before surfacing the failure
        renderer.reset();
        window.reset();

        TTF_Quit();
        Mix_CloseAudio();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();

        throw;
    }
}

/** Destroy the active renderer and window, then stop SDL subsystems */
template <typename WindowPointer, typename RendererPointer>
void end(WindowPointer &window, RendererPointer &renderer) {
    spdlog::info("Stopping SDL lifecycle");

    renderer.reset();
    window.reset();

    TTF_Quit();
    spdlog::debug("Stopped SDL_ttf");

    Mix_CloseAudio();
    spdlog::debug("Closed SDL_mixer audio");

    Mix_Quit();
    spdlog::debug("Stopped SDL_mixer");

    IMG_Quit();
    spdlog::debug("Stopped SDL_image");

    SDL_Quit();
    spdlog::debug("Stopped SDL");
}

} // namespace Engine::Lifecycle
