#pragma once

#include <CLI/CLI.hpp>

#include <filesystem>
#include <memory>
#include <string>

namespace Arguments {

/** Keep CLI help compact and avoid repeating validator details in options */
class CompactHelpFormatter : public CLI::Formatter {
  public:
    std::string make_option_opts(const CLI::Option *option) const override {
        for(const std::string &name : option->get_lnames()) {
            if(name == "--log-level" || name == "log-level") {
                return "";
            }
        }

        return CLI::Formatter::make_option_opts(option);
    }

    std::string make_help(
        const CLI::App *application,
        std::string name,
        CLI::AppFormatMode mode
    ) const override {
        std::string help = CLI::Formatter::make_help(application, name, mode);
        std::string::size_type position = help.find("\n\n");

        while(position != std::string::npos) {
            help.replace(position, 2, "\n");
            position = help.find("\n\n");
        }

        return help;
    }
};

/** Parsed command-line values used to configure startup systems */
struct Parsed {
    /** Path used to launch the executable */
    std::filesystem::path executablePath;

    /** Console log level name; "off" disables console logging */
    std::string logLevel = "off";
};

/**
 * Register supported command-line options and parse process arguments
 *
 * @param application CLI11 application that owns option definitions and exit
 * handling
 * @param argumentCount Number of process arguments
 * @param arguments Process argument vector
 * @return Parsed startup arguments
 *
 * @throws CLI::ParseError when CLI11 handles help, validation, or parse
 * errors; callers should pass the exception back to CLI::App::exit()
 */
inline Parsed
parse(CLI::App &application, int argumentCount, const char *arguments[]) {
    Parsed parsedArguments;
    parsedArguments.executablePath = arguments[0];

    application.formatter(std::make_shared<CompactHelpFormatter>());
    application.set_help_flag(
        "-h,--help",
        "Print this help message and exit."
    );

    application
        .add_option(
            "--log-level",
            parsedArguments.logLevel,
            "Log level: trace, debug, info, warn, error, critical, off."
        )
        ->check(
            CLI::IsMember(
                {"trace", "debug", "info", "warn", "error", "critical", "off"}
            )
        );

    application.parse(argumentCount, arguments);

    return parsedArguments;
}

} // namespace Arguments
