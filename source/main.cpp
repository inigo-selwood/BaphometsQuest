#include "cli.hpp"
#include "engine/components/box.hpp"
#include "engine/components/text.hpp"
#include "engine/components/texture.hpp"
#include "engine/components/timer.hpp"
#include "engine/game/game.hpp"
#include "game/nodes/clickableRedBox.hpp"
#include "game/nodes/colourChangeBox.hpp"
#include "game/scenes/mainScene.hpp"
#include "logging.hpp"

#include <exception>
#include <optional>
#include <spdlog/spdlog.h>

int main(int argumentCount, const char *arguments[]) {
    ApplicationArguments applicationArguments;

    const std::optional<int> argumentExitCode =
        parseArguments(argumentCount, arguments, applicationArguments);

    if(argumentExitCode.has_value()) {
        return *argumentExitCode;
    }

    configureLogger(
        applicationArguments.consoleLoggingEnabled,
        arguments[0],
        applicationArguments.logLevel
    );

    spdlog::info(
        "Starting Baphomet's Quest with {} argument(s).",
        argumentCount
    );

    int exitCode = 0;
    auto &game = Engine::Game::getInstance();
    bool gameSystemsMayNeedQuit = false;

    try {
        // Register XML-loadable node types available to every application
        // mode.
        Box::registerType();
        ClickableRedBox::registerType();
        ColourChangeBox::registerType();
        Text::registerType();
        Engine::Timer::registerType();
        Texture::registerType();

        switch(applicationArguments.mode) {
        case ApplicationMode::Play:
            gameSystemsMayNeedQuit = true;
            game.registerScene<MainScene>("main");

            game.queueScene("main");
            game.start();
            game.run();
            break;
        case ApplicationMode::Tileset:
            spdlog::error("Tileset editor mode is not implemented yet.");
            exitCode = 1;
            break;
        case ApplicationMode::MapEdit:
            spdlog::error("Map editor mode is not implemented yet.");
            exitCode = 1;
            break;
        }
    } catch(const std::exception &exception) {
        spdlog::error("{}", exception.what());
        exitCode = 1;
    } catch(...) {
        spdlog::error("Unhandled unknown exception.");
        exitCode = 1;
    }

    if(gameSystemsMayNeedQuit) {
        game.quit();
    }

    return exitCode;
}
