#include "game.hpp"

#include "../../logger.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>

namespace Engine {

namespace {

constexpr char SETTINGS_PATH[] = "resources/configuration/settings.yaml";

template <typename Value>
Value readSetting(
    const ::YAML::Node &settings,
    std::initializer_list<std::string_view> path,
    const Value &fallback
) {
    ::YAML::Node value = settings;
    std::string keyPath;

    for(const std::string_view key : path) {
        if(!keyPath.empty()) {
            keyPath += ".";
        }

        keyPath += key;
        value = value[std::string(key)];

        if(!value) {
            return fallback;
        }
    }

    try {
        return value.as<Value>();
    } catch(const ::YAML::Exception &exception) {
        throw std::runtime_error(
            "Invalid setting '" + keyPath + "': " + exception.what()
        );
    }
}

} // namespace

Game::~Game() {
    Engine::Lifecycle::end(this->window, this->renderer);
}

void Game::start(
    const std::filesystem::path &executablePath,
    const std::string &consoleLogLevel
) {
    this->settingsId =
        this->resources.load<Engine::Resource::YAML>(SETTINGS_PATH);

    const ::YAML::Node &settings =
        *this->resources.get<Engine::Resource::YAML>(this->settingsId).node;

    Logger::start(
        executablePath,
        consoleLogLevel,
        readSetting<std::string>(
            settings,
            {"logger", "application-name"},
            "baphomets_quest"
        )
    );

    spdlog::info("Starting game services");

    Engine::Lifecycle::start(
        this->window,
        this->renderer,
        this->resources.get<Engine::Resource::YAML>(this->settingsId)
    );
}

void Game::run() {
    if(this->settingsId == 0 || this->renderer == nullptr) {
        throw std::runtime_error("Game must be started before it can run");
    }

    spdlog::info("Starting game run stub");

    SDL_SetRenderDrawColor(this->renderer.get(), 0, 0, 0, 255);
    SDL_RenderClear(this->renderer.get());
    SDL_RenderPresent(this->renderer.get());

    spdlog::info("Finished game run stub");
}

} // namespace Engine
