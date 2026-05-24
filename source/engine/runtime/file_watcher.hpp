#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

/** Polling file watcher used for lightweight development hot reloads */
class FileWatcher {
  public:
    /** Replace the watched file set and record current timestamps */
    void watch(const std::vector<std::string> &paths);

    /** Return true when a watched file has changed since the last poll */
    bool hasChanged();

  private:
    using Clock = std::chrono::steady_clock;

    static constexpr std::chrono::milliseconds POLL_INTERVAL{250};

    /** Return a stable display path for lookup and logging */
    static std::string normalizePath(const std::string &path);

    std::unordered_map<std::string, std::filesystem::file_time_type> files;
    Clock::time_point lastCheckedAt = Clock::time_point::min();
};

} // namespace Engine
