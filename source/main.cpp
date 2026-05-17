#include "arguments.hpp"
#include "logger.hpp"

#include <CLI/CLI.hpp>

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

    return 0;
}
