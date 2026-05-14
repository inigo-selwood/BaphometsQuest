#include "game.hpp"

namespace Engine {

Game &Game::getInstance() {
    static Game game;
    return game;
}

AssetRegistry &Game::getAssets() {
    return this->assets;
}

const InputState &Game::getInput() const {
    return this->input;
}

void Game::queueQuit() {
    spdlog::info("Queueing game quit.");
    this->running = false;
}

} // namespace Engine
