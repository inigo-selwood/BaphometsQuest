#include "game.hpp"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

namespace Engine {

void Game::run() {
    spdlog::info("Starting game run stub");

    this->Window.reset(SDL_CreateWindow(
        "Baphomet's Quest",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480,
        SDL_WINDOW_SHOWN
    ));

    if(this->Window == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create window: ") + SDL_GetError()
        );
    }

    this->Renderer.reset(
        SDL_CreateRenderer(this->Window.get(), -1, SDL_RENDERER_ACCELERATED)
    );

    if(this->Renderer == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create renderer: ") + SDL_GetError()
        );
    }

    SDL_SetRenderDrawColor(this->Renderer.get(), 0, 0, 0, 255);
    SDL_RenderClear(this->Renderer.get());
    SDL_RenderPresent(this->Renderer.get());

    spdlog::info("Finished game run stub");
}

} // namespace Engine
