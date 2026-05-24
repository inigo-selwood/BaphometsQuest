#include "file_watcher.hpp"

#include <system_error>

namespace Engine {

void FileWatcher::watch(const std::vector<std::string> &paths) {
    this->files.clear();
    this->lastCheckedAt = Clock::now();

    for(const std::string &path : paths) {
        std::error_code error;
        const std::string normalizedPath = normalizePath(path);
        const std::filesystem::file_time_type lastWriteTime =
            std::filesystem::last_write_time(normalizedPath, error);

        if(error) {
            this->files.emplace(
                normalizedPath,
                std::filesystem::file_time_type::min()
            );
            continue;
        }

        this->files.emplace(normalizedPath, lastWriteTime);
    }
}

bool FileWatcher::hasChanged() {
    const Clock::time_point now = Clock::now();

    if(now - this->lastCheckedAt < POLL_INTERVAL) {
        return false;
    }

    this->lastCheckedAt = now;

    for(auto &[path, previousWriteTime] : this->files) {
        std::error_code error;
        const std::filesystem::file_time_type lastWriteTime =
            std::filesystem::last_write_time(path, error);

        if(error) {
            if(previousWriteTime != std::filesystem::file_time_type::min()) {
                previousWriteTime = std::filesystem::file_time_type::min();
                return true;
            }

            continue;
        }

        if(lastWriteTime != previousWriteTime) {
            previousWriteTime = lastWriteTime;
            return true;
        }
    }

    return false;
}

std::string FileWatcher::normalizePath(const std::string &path) {
    std::error_code error;
    const std::filesystem::path normalizedPath =
        std::filesystem::weakly_canonical(path, error);

    if(error) {
        return std::filesystem::path{path}.lexically_normal().generic_string();
    }

    return normalizedPath.generic_string();
}

} // namespace Engine
