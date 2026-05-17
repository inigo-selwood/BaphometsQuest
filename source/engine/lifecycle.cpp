#include "lifecycle.hpp"

#include <stdexcept>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <spdlog/spdlog.h>

namespace Engine::Lifecycle {

Session::Session() {
    start();
}

Session::~Session() {
    stop();
}

void start() {
    spdlog::info("Starting SDL lifecycle");

    try {
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

        if(TTF_Init() != 0) {
            throw std::runtime_error(
                std::string("Failed to initialize SDL_ttf: ") + TTF_GetError()
            );
        }

        spdlog::debug("Initialized SDL_ttf");
    } catch(...) {
        stop();
        throw;
    }
}

void stop() {
    spdlog::info("Stopping SDL lifecycle");

    TTF_Quit();
    spdlog::debug("Stopped SDL_ttf");

    Mix_Quit();
    spdlog::debug("Stopped SDL_mixer");

    IMG_Quit();
    spdlog::debug("Stopped SDL_image");

    SDL_Quit();
    spdlog::debug("Stopped SDL");
}

} // namespace Engine::Lifecycle
