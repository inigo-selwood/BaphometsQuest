#include "arguments.hpp"
#include "engine/game.hpp"
#include "engine/lifecycle.hpp"
#include "logger.hpp"

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

    Logger::start(parsedArguments.executablePath, parsedArguments.logLevel);

    try {
        Engine::Lifecycle::Session lifecycle;

        Engine::Game game;
        game.run();
    } catch(const std::exception &exception) {
        spdlog::error("{}", exception.what());
        return 1;
    }

    return 0;
}
