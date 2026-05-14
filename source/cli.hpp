#pragma once

#include <CLI/CLI.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

enum class ApplicationMode {
    Play,
    Tileset,
    MapEdit,
};

struct ApplicationArguments {
    bool consoleLoggingEnabled = false;
    std::string logLevel = "off";
    std::string mapEditPath;
    std::string tilesetPath;
    ApplicationMode mode = ApplicationMode::Play;
};

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

/** Parse command-line mode and mode-specific flags. */
inline std::optional<int> parseArguments(
    int argumentCount,
    const char *arguments[],
    ApplicationArguments &applicationArguments
) {
    CLI::App application{"Baphomet's Quest"};
    application.formatter(std::make_shared<CompactHelpFormatter>());

    const auto logLevelOptions = CLI::IsMember(
        {"trace", "debug", "info", "warn", "error", "critical", "off"}
    );
    std::vector<CLI::Option *> logLevelOptionsUsed;
    const auto addLogLevelOption = [&](CLI::App *app) {
        CLI::Option *option =
            app->add_option(
                   "--log-level",
                   applicationArguments.logLevel,
                   "Log level: trace, debug, info, warn, error, critical, off."
            )
                ->check(logLevelOptions);
        logLevelOptionsUsed.push_back(option);
    };

    application.require_subcommand(0, 1);
    addLogLevelOption(&application);

    CLI::App *play = application.add_subcommand("play", "Run gameplay mode.");
    addLogLevelOption(play);
    play->callback([&applicationArguments] {
        applicationArguments.mode = ApplicationMode::Play;
    });

    CLI::App *tileset =
        application.add_subcommand("tileset", "Run tileset editor mode.");
    addLogLevelOption(tileset);
    tileset->add_option(
        "--file",
        applicationArguments.tilesetPath,
        "Tileset image path."
    );
    tileset->callback([&applicationArguments] {
        applicationArguments.consoleLoggingEnabled = true;
        applicationArguments.mode = ApplicationMode::Tileset;
    });

    CLI::App *mapEdit =
        application.add_subcommand("map-edit", "Run map editor mode.");
    addLogLevelOption(mapEdit);
    mapEdit->add_option(
        "--map",
        applicationArguments.mapEditPath,
        "Map data path."
    );
    mapEdit->callback([&applicationArguments] {
        applicationArguments.consoleLoggingEnabled = true;
        applicationArguments.mode = ApplicationMode::MapEdit;
    });

    try {
        application.parse(argumentCount, arguments);
    } catch(const CLI::ParseError &exception) {
        return application.exit(exception);
    }

    for(const CLI::Option *option : logLevelOptionsUsed) {
        if(option->count() > 0 && applicationArguments.logLevel != "off") {
            applicationArguments.consoleLoggingEnabled = true;
            break;
        }
    }

    return std::nullopt;
}
