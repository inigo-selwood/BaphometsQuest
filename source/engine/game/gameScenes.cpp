#include "game.hpp"

namespace Engine {

void Game::queueScene(const std::string &name) {
    if(!this->sceneFactories.contains(name)) {
        throw std::runtime_error("Unknown scene: " + name);
    }

    spdlog::info("Queueing scene '{}'.", name);
    this->queuedSceneName = name;
}

void Game::loadQueuedScene() {
    if(!this->queuedSceneName.has_value()) {
        return;
    }

    if(this->activeScene != nullptr) {
        this->destroyActiveScene();
        this->unloadActiveSceneAssets();
    }

    spdlog::info("Loading scene '{}'.", *this->queuedSceneName);
    this->activeScene = this->sceneFactories.at(*this->queuedSceneName)();
    this->activeScene->enterTree();
    spdlog::info("Scene '{}' loaded.", *this->queuedSceneName);
    this->queuedSceneName.reset();
}

void Game::destroyActiveScene() {
    if(this->activeScene == nullptr) {
        return;
    }

    spdlog::debug("Exiting active scene.");
    this->activeScene->exitTree();
    this->activeScene.reset();
}

void Game::unloadActiveSceneAssets() {
    this->assets.unloadAll(AssetRegistry::AssetType::ImageTexture);
    this->assets.unloadAll(AssetRegistry::AssetType::TextTexture);
}

} // namespace Engine
