#include "game/scenes/main.hpp"
#include "core/arguments.hpp"
#include "engine/runtime/game.hpp"

#include <CLI/CLI.hpp>

#include <exception>
#include <memory>
#include <spdlog/spdlog.h>

int main(int argumentCount, const char *arguments[]) {
    CLI::App application{"Baphomet's Quest"};
    Arguments::Parsed parsedArguments;

    try {
        parsedArguments =
            Arguments::parse(application, argumentCount, arguments);
    } catch(const CLI::ParseError &exception) {
        return application.exit(exception);
    }

    try {
        auto game = std::make_shared<Engine::Game>();
        game->start(parsedArguments.executablePath, parsedArguments.logLevel);
        game->registerScene<Game::Scenes::Main>("main");
        game->queueScene("main");
        game->run();
    } catch(const std::exception &exception) {
        spdlog::error("{}", exception.what());
        return 1;
    }

    return 0;
}
