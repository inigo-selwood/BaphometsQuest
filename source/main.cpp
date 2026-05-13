#include "components/box.hpp"
#include "components/text.hpp"
#include "components/texture.hpp"
#include "engine/runtime/game.hpp"
#include "game/nodes/clickableRedBox.hpp"
#include "game/nodes/colourChangeBox.hpp"
#include "scenes/mainScene.hpp"

#include <CLI/CLI.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace {

const std::size_t MAX_LOG_FILES = 3;

enum class ApplicationMode {
    Play,
    Tileset,
    Tilemap,
};

struct ApplicationArguments {
    bool consoleLoggingEnabled = false;
    std::string logLevel = "debug";
    std::string tilemapPath;
    std::string tilesetPath;
    ApplicationMode mode = ApplicationMode::Play;
};

/** Return the spdlog level matching a parsed command-line level. */
spdlog::level::level_enum getLogLevel(const std::string &name) {
    if(name == "trace") {
        return spdlog::level::trace;
    }

    if(name == "debug") {
        return spdlog::level::debug;
    }

    if(name == "info") {
        return spdlog::level::info;
    }

    if(name == "warn") {
        return spdlog::level::warn;
    }

    if(name == "error") {
        return spdlog::level::err;
    }

    if(name == "critical") {
        return spdlog::level::critical;
    }

    return spdlog::level::off;
}

/** Configure the shared logger before game systems start using it. */
void configureLogger(
    bool consoleLoggingEnabled,
    const std::string &executablePath,
    const std::string &logLevel
) {
    spdlog::set_pattern("[%T] [%^%-5l%$]: %v");
    spdlog::flush_on(spdlog::level::warn);

    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};

    localtime_r(&time, &localTime);

    std::ostringstream timestamp;
    timestamp << std::put_time(&localTime, "%Y-%m-%d %H-%M-%S");

    const std::filesystem::path executableDirectory =
        std::filesystem::weakly_canonical(
            std::filesystem::absolute(executablePath)
        )
            .parent_path();
    const std::filesystem::path logsDirectory = executableDirectory / ".logs";

    std::filesystem::create_directories(logsDirectory);

    std::vector<std::filesystem::directory_entry> logFiles;

    for(const auto &entry :
        std::filesystem::directory_iterator(logsDirectory)) {
        if(entry.is_regular_file() && entry.path().extension() == ".txt") {
            logFiles.push_back(entry);
        }
    }

    std::sort(
        logFiles.begin(),
        logFiles.end(),
        [](const auto &left, const auto &right) {
            return left.last_write_time() > right.last_write_time();
        }
    );

    for(std::size_t index = MAX_LOG_FILES - 1; index < logFiles.size();
        index++) {
        std::filesystem::remove(logFiles[index]);
    }

    const std::filesystem::path logPath =
        logsDirectory / (timestamp.str() + ".txt");

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        logPath.string(),
        false
    );
    std::vector<spdlog::sink_ptr> logSinks{fileSink};

    if(consoleLoggingEnabled) {
        logSinks.push_back(
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
        );
    }

    spdlog::set_default_logger(
        std::make_shared<spdlog::logger>(
            "untitled_game",
            logSinks.begin(),
            logSinks.end()
        )
    );
    spdlog::set_pattern("[%T] [%^%-5l%$]: %v");
    spdlog::set_level(getLogLevel(logLevel));
    spdlog::flush_on(spdlog::level::warn);
    spdlog::info("Logging to '{}'.", logPath.string());
}

/** Parse command-line mode and mode-specific flags. */
std::optional<int> parseArguments(
    int argumentCount,
    const char *arguments[],
    ApplicationArguments &applicationArguments
) {
    CLI::App application{"Untitled Game"};
    const auto logLevelOptions = CLI::IsMember(
        {"trace", "debug", "info", "warn", "error", "critical", "off"}
    );
    const auto addLogLevelOption = [&](CLI::App *app) {
        app->add_option(
               "--log-level",
               applicationArguments.logLevel,
               "Log level: trace, debug, info, warn, error, critical, off."
        )
            ->check(logLevelOptions);
    };

    application.require_subcommand(0, 1);
    addLogLevelOption(&application);

    CLI::App *play = application.add_subcommand("play", "Run gameplay mode.");
    addLogLevelOption(play);
    play->callback([&applicationArguments] {
        applicationArguments.mode = ApplicationMode::Play;
    });

    CLI::App *debug =
        application.add_subcommand("debug", "Run gameplay mode with logging.");
    addLogLevelOption(debug);
    debug->callback([&applicationArguments] {
        applicationArguments.consoleLoggingEnabled = true;
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

    CLI::App *tilemap =
        application.add_subcommand("tilemap", "Run tilemap editor mode.");
    addLogLevelOption(tilemap);
    tilemap->add_option(
        "--map",
        applicationArguments.tilemapPath,
        "Tilemap data path."
    );
    tilemap->callback([&applicationArguments] {
        applicationArguments.consoleLoggingEnabled = true;
        applicationArguments.mode = ApplicationMode::Tilemap;
    });

    try {
        application.parse(argumentCount, arguments);
    } catch(const CLI::ParseError &exception) {
        return application.exit(exception);
    }

    return std::nullopt;
}

} // namespace

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

    spdlog::info("Starting Untitled Game with {} argument(s).", argumentCount);

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
            spdlog::error("Tileset editor mode is not implemented yet.");
            exitCode = 1;
            break;
        case ApplicationMode::Tilemap:
            spdlog::error("Tilemap editor mode is not implemented yet.");
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
