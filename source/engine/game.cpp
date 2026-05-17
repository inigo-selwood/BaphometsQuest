#include "game.hpp"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

namespace Engine {

void Game::run() {
    spdlog::info("Starting game run stub");

    this->window.reset(SDL_CreateWindow(
        "Baphomet's Quest",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480,
        SDL_WINDOW_SHOWN
    ));

    if(this->window == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create window: ") + SDL_GetError()
        );
    }

    this->renderer.reset(
        SDL_CreateRenderer(this->window.get(), -1, SDL_RENDERER_ACCELERATED)
    );

    if(this->renderer == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create renderer: ") + SDL_GetError()
        );
    }

    SDL_SetRenderDrawColor(this->renderer.get(), 0, 0, 0, 255);
    SDL_RenderClear(this->renderer.get());
    SDL_RenderPresent(this->renderer.get());

    spdlog::info("Finished game run stub");
}

} // namespace Engine
