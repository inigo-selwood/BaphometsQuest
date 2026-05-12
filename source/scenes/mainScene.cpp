#include "mainScene.hpp"

#include "../engine/sceneLoader.hpp"
#include "../specialized/clickableRedBox.hpp"
#include "../specialized/colourChangeBox.hpp"

void MainScene::onEnterTree() {
    spdlog::info("Main scene entered.");
    loadSceneFromXML(*this, "resources/scenes/main.xml");

    auto &blueBox = getChild<ColourChangeBox>("blueBox");
    auto &redBox = blueBox.getChild<ClickableRedBox>("redBox");

    redBox.connectSignal<int, int, int>(
            "clicked", [&blueBox](int button, int x, int y) {
                spdlog::info("Red box clicked at {},{} with button {}; "
                             "turning blue box green.",
                        x,
                        y,
                        button);
                blueBox.turnGreen();
            });
}
