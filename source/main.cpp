#include "components/box.hpp"
#include "components/text.hpp"
#include "components/texture.hpp"
#include "engine/game.hpp"
#include "engine/includes.hpp"
#include "scenes/mainScene.hpp"
#include "specialized/clickableRedBox.hpp"
#include "specialized/colourChangeBox.hpp"

#include <exception>

int main(int argumentCount, const char *arguments[]) {
    (void)arguments;

    // Configure the shared logger before game systems start using it.
    spdlog::set_pattern("[%T] [%^%l%$] %n: %v");
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::warn);

    spdlog::info("Starting Untitled Game with {} argument(s).", argumentCount);

    int exitCode = 0;
    auto &game = Game::getInstance();

    try {
        game.registerScene<MainScene>("main");
        Box::registerType();
        ClickableRedBox::registerType();
        ColourChangeBox::registerType();
        Text::registerType();
        Texture::registerType();
        game.queueScene("main");
        game.start();
        game.run();
    } catch (const std::exception &exception) {
        spdlog::error("{}", exception.what());
        exitCode = 1;
    } catch (...) {
        spdlog::error("Unhandled unknown exception.");
        exitCode = 1;
    }

    game.quit();

    return exitCode;
}
