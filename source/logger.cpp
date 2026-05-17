#include "logger.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Logger {

namespace {

constexpr std::size_t MAX_LOG_FILES = 3;

/** Return a filesystem-safe ISO-like timestamp for log filenames */
std::string getISOTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};

    localtime_r(&time, &localTime);

    std::ostringstream timestamp;
    timestamp << std::put_time(&localTime, "%Y-%m-%dT%H-%M-%S");

    return timestamp.str();
}

/** Keep space tidy by retaining only the newest log files */
void pruneLogs(const std::filesystem::path &logsDirectory) {
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

    const std::size_t logsToKeepBeforeCurrent = MAX_LOG_FILES - 1;

    // Keep room for the log file that will be created during this startup
    for(std::size_t index = logsToKeepBeforeCurrent; index < logFiles.size();
        index++) {
        std::filesystem::remove(logFiles[index]);
    }
}

/** Convert a command-line log level name into spdlog's enum */
spdlog::level::level_enum parseLevel(const std::string &level) {
    if(level == "trace") {
        return spdlog::level::trace;
    }

    if(level == "debug") {
        return spdlog::level::debug;
    }

    if(level == "info") {
        return spdlog::level::info;
    }

    if(level == "warn") {
        return spdlog::level::warn;
    }

    if(level == "error") {
        return spdlog::level::err;
    }

    if(level == "critical") {
        return spdlog::level::critical;
    }

    return spdlog::level::off;
}

} // namespace

void start(
    const std::filesystem::path &executablePath,
    const std::string &consoleLevel,
    const std::string &applicationName
) {
    const spdlog::level::level_enum consoleLogLevel = parseLevel(consoleLevel);

    // Logs live beside the executable so debug output follows each build
    const std::filesystem::path executableDirectory =
        std::filesystem::weakly_canonical(
            std::filesystem::absolute(executablePath)
        )
            .parent_path();
    const std::filesystem::path logsDirectory = executableDirectory / ".logs";
    std::filesystem::create_directories(logsDirectory);
    pruneLogs(logsDirectory);

    const std::filesystem::path logPath =
        logsDirectory / (getISOTimestamp() + ".txt");

    // File logs are always trace-level; the CLI only controls console noise
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        logPath.string(),
        false
    );
    fileSink->set_level(spdlog::level::trace);

    std::vector<spdlog::sink_ptr> sinks{fileSink};

    if(consoleLogLevel != spdlog::level::off) {
        auto consoleSink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(consoleLogLevel);
        sinks.push_back(consoleSink);
    }

    // The logger itself stays at trace so sink levels decide what is emitted
    auto logger = std::make_shared<spdlog::logger>(
        applicationName,
        sinks.begin(),
        sinks.end()
    );
    logger->set_level(spdlog::level::trace);

    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%T] [%^%-5l%$]: %v");
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::warn);

    spdlog::info("Logging to '{}'.", logPath.string());
}

} // namespace Logger
