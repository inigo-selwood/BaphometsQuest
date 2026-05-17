#pragma once

#include "../resource/types/yaml.hpp"

#include <stdexcept>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <spdlog/spdlog.h>

namespace Engine::Lifecycle {

/** Start SDL subsystems and create the active window and renderer */
template <typename WindowPointer, typename RendererPointer>
void start(
    WindowPointer &window,
    RendererPointer &renderer,
    const Engine::Resource::YAML &settings
) {
    try {
        spdlog::info("Starting SDL lifecycle");

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

        if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
            throw std::runtime_error(
                std::string("Failed to initialize SDL_mixer: ")
                + Mix_GetError()
            );
        }

        spdlog::debug("Initialized SDL_mixer with OGG support");

        if(Mix_OpenAudio(
               (*settings.node)["audio"]["frequency"].as<int>(),
               MIX_DEFAULT_FORMAT,
               (*settings.node)["audio"]["channels"].as<int>(),
               (*settings.node)["audio"]["chunk-size"].as<int>()
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to open SDL_mixer audio: ")
                + Mix_GetError()
            );
        }

        spdlog::debug("Opened SDL_mixer audio");

        if(TTF_Init() != 0) {
            throw std::runtime_error(
                std::string("Failed to initialize SDL_ttf: ") + TTF_GetError()
            );
        }

        spdlog::debug("Initialized SDL_ttf");

        const std::string windowTitle =
            (*settings.node)["window"]["title"].as<std::string>();

        window.reset(SDL_CreateWindow(
            windowTitle.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            (*settings.node)["window"]["width"].as<int>(),
            (*settings.node)["window"]["height"].as<int>(),
            SDL_WINDOW_SHOWN
        ));

        if(window == nullptr) {
            throw std::runtime_error(
                std::string("Failed to create window: ") + SDL_GetError()
            );
        }

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

        const int rendererScale =
            (*settings.node)["renderer"]["scale"].as<int>();

        if(SDL_RenderSetScale(
               renderer.get(),
               static_cast<float>(rendererScale),
               static_cast<float>(rendererScale)
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to set renderer scale: ") + SDL_GetError()
            );
        }
    } catch(...) {
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
