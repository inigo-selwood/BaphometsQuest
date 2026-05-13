#include "components/box.hpp"
#include "components/text.hpp"
#include "components/texture.hpp"
#include "engine/runtime/game.hpp"
#include "game/nodes/clickableRedBox.hpp"
#include "game/nodes/colourChangeBox.hpp"
#include "scenes/mainScene.hpp"

#include <CLI/CLI.hpp>

#include <chrono>
#include <ctime>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace {

enum class ApplicationMode {
    Play,
    Tileset,
    Tilemap,
};

struct ApplicationArguments {
    bool loggingEnabled = false;
    std::string tilemapPath;
    std::string tilesetPath;
    ApplicationMode mode = ApplicationMode::Play;
};

} // namespace

int main(int argumentCount, const char *arguments[]) {
    ApplicationArguments applicationArguments;
    CLI::App application{"Untitled Game"};

    // Parse command-line mode and mode-specific flags.
    application.require_subcommand(0, 1);

    CLI::App *play = application.add_subcommand("play", "Run gameplay mode.");
    play->callback([&applicationArguments] {
        applicationArguments.mode = ApplicationMode::Play;
    });

    CLI::App *debug =
        application.add_subcommand("debug", "Run gameplay mode with logging.");
    debug->callback([&applicationArguments] {
        applicationArguments.loggingEnabled = true;
        applicationArguments.mode = ApplicationMode::Play;
    });

    CLI::App *tileset =
        application.add_subcommand("tileset", "Run tileset editor mode.");
    tileset->add_option(
        "--file",
        applicationArguments.tilesetPath,
        "Tileset image path."
    );
    tileset->callback([&applicationArguments] {
        applicationArguments.loggingEnabled = true;
        applicationArguments.mode = ApplicationMode::Tileset;
    });

    CLI::App *tilemap =
        application.add_subcommand("tilemap", "Run tilemap editor mode.");
    tilemap->add_option(
        "--map",
        applicationArguments.tilemapPath,
        "Tilemap data path."
    );
    tilemap->callback([&applicationArguments] {
        applicationArguments.loggingEnabled = true;
        applicationArguments.mode = ApplicationMode::Tilemap;
    });

    try {
        application.parse(argumentCount, arguments);
    } catch(const CLI::ParseError &exception) {
        return application.exit(exception);
    }

    // Configure the shared logger before game systems start using it.
    spdlog::set_pattern("[%T] [%^%-5l%$]: %v");
    spdlog::flush_on(spdlog::level::warn);

    if(applicationArguments.loggingEnabled) {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm localTime{};

        localtime_r(&time, &localTime);

        std::ostringstream timestamp;
        timestamp << std::put_time(&localTime, "%Y-%m-%d %H-%M-%S");

        const std::filesystem::path executableDirectory =
            std::filesystem::weakly_canonical(
                std::filesystem::absolute(arguments[0])
            )
                .parent_path();
        const std::filesystem::path logsDirectory =
            executableDirectory / ".logs";
        const std::filesystem::path logPath =
            logsDirectory / (timestamp.str() + ".txt");

        std::filesystem::create_directories(logsDirectory);
        spdlog::set_default_logger(
            spdlog::basic_logger_mt("untitled_game", logPath.string())
        );
        spdlog::set_pattern("[%T] [%^%-5l%$]: %v");
        spdlog::set_level(spdlog::level::debug);
        spdlog::flush_on(spdlog::level::warn);
        spdlog::info("Logging to '{}'.", logPath.string());
    } else {
        spdlog::set_level(spdlog::level::off);
    }

    if(applicationArguments.loggingEnabled) {
        spdlog::info(
            "Starting Untitled Game with {} argument(s).",
            argumentCount
        );
    }

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
            if(applicationArguments.loggingEnabled) {
                spdlog::error("Tileset editor mode is not implemented yet.");
            } else {
                std::cerr << "Tileset editor mode is not implemented yet.\n";
            }

            exitCode = 1;
            break;
        case ApplicationMode::Tilemap:
            if(applicationArguments.loggingEnabled) {
                spdlog::error("Tilemap editor mode is not implemented yet.");
            } else {
                std::cerr << "Tilemap editor mode is not implemented yet.\n";
            }

            exitCode = 1;
            break;
        }
    } catch(const std::exception &exception) {
        if(applicationArguments.loggingEnabled) {
            spdlog::error("{}", exception.what());
        } else {
            std::cerr << exception.what() << '\n';
        }

        exitCode = 1;
    } catch(...) {
        if(applicationArguments.loggingEnabled) {
            spdlog::error("Unhandled unknown exception.");
        } else {
            std::cerr << "Unhandled unknown exception.\n";
        }

        exitCode = 1;
    }

    if(gameSystemsMayNeedQuit) {
        game.quit();
    }

    return exitCode;
}
