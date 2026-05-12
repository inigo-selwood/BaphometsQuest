#include "mainScene.hpp"

#include "../engine/sceneLoader.hpp"

void MainScene::onEnterTree() {
    spdlog::info("Main scene entered.");
    loadSceneFromXML(*this, "resources/scenes/main.xml");
}
