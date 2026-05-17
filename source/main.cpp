#include "arguments.hpp"
#include "engine/game.hpp"

#include <CLI/CLI.hpp>

#include <exception>
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
        Engine::Game game;
        game.start(parsedArguments.executablePath, parsedArguments.logLevel);
        game.run();
    } catch(const std::exception &exception) {
        spdlog::error("{}", exception.what());
        return 1;
    }

    return 0;
}
