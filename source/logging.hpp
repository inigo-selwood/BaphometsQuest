#pragma once

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace {

const std::size_t MAX_LOG_FILES = 3;

/** Return the spdlog level matching a parsed command-line level. */
inline spdlog::level::level_enum getLogLevel(const std::string &name) {
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

} // namespace

/** Configure the shared logger before game systems start using it. */
inline void configureLogger(
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
    fileSink->set_level(spdlog::level::trace);

    std::vector<spdlog::sink_ptr> logSinks{fileSink};

    if(consoleLoggingEnabled) {
        auto consoleSink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(getLogLevel(logLevel));
        logSinks.push_back(consoleSink);
    }

    spdlog::set_default_logger(
        std::make_shared<spdlog::logger>(
            "baphomets_quest",
            logSinks.begin(),
            logSinks.end()
        )
    );
    spdlog::set_pattern("[%T] [%^%-5l%$]: %v");
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::warn);
    spdlog::info("Logging to '{}'.", logPath.string());
}
