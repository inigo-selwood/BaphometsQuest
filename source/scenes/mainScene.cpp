#include "mainScene.hpp"

#include "../engine/runtime/game.hpp"
#include "../engine/scene/sceneLoader.hpp"
#include "../game/nodes/clickableRedBox.hpp"
#include "../game/nodes/colourChangeBox.hpp"

void MainScene::onEnterTree() {
    spdlog::info("Main scene entered.");
    Engine::loadSceneFromXML(
        *this,
        Engine::Game::getInstance().getAssets(),
        "resources/scenes/main.xml"
    );

    auto &blueBox = this->getChild<ColourChangeBox>("blueBox");
    auto &redBox = blueBox.getChild<ClickableRedBox>("redBox");

    redBox.connectSignal<int, int, int>(
        "clicked",
        [&blueBox](int button, int x, int y) {
            spdlog::info(
                "Red box clicked at {},{} with button {}; "
                "turning blue box green.",
                x,
                y,
                button
            );
            blueBox.turnGreen();
        }
    );
}
