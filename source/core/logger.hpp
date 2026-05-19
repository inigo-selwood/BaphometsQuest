#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace Logger {

/**
 * Configure the default spdlog logger
 *
 * File logs are always written at trace level to
 * `{build profile}/.logs/{ISO timestamp}.txt` when running from a build
 * directory, with only the three most recent log files kept. Console logging
 * is disabled when @p consoleLevel is "off"; otherwise the console sink uses
 * the requested level
 *
 * Logging convention:
 * - info: events that happen once
 * - debug: events that happen repeatedly, but not every cycle
 * - trace: events that may happen more than once per cycle
 *
 * @param executablePath Path used to launch the executable
 * @param consoleLevel Console log level name: trace, debug, info, warn, error,
 * critical, or off
 */
void start(
    const std::filesystem::path &executablePath,
    const std::string &consoleLevel
);

/** Indent multiline payloads to align with the logger message column */
std::string indentPayload(std::string_view text);

} // namespace Logger
