#include "support.hpp"

#include "engine/runtime/state.hpp"

#include <SDL.h>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>

TEST_CASE("state store handles flags and typed values") {
    Engine::State::Store state;

    CHECK_FALSE(state.getFlag("met-priest"));
    state.setFlag("met-priest");
    CHECK(state.getFlag("met-priest"));

    state.set("coins", 7);
    state.set("current-map", std::string{"home-town"});
    state.set("player-position", SDL_Point{80, 72});

    CHECK(state.get<int>("coins") == 7);
    CHECK(state.get<std::string>("current-map") == "home-town");
    Tests::checkPoint(
        state.get<SDL_Point>("player-position"),
        SDL_Point{80, 72}
    );

    CHECK(state.ensure("health", 3) == 3);
    CHECK(state.has("health"));
    state.erase("health");
    CHECK_FALSE(state.has("health"));
}

TEST_CASE("state store fails loudly for missing or mismatched typed values") {
    Engine::State::Store state;

    state.set("coins", 7);

    CHECK_THROWS_AS(state.get<std::string>("coins"), std::runtime_error);
    CHECK_THROWS_AS(state.get<int>("missing"), std::runtime_error);
}

TEST_CASE("state store persists YAML without explicit type tags") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "state.yaml";
    Engine::State::Store state;

    state.set("current-map", std::string{"home-town"});
    state.set("player-position", SDL_Point{80, 72});
    state.save(path);

    Engine::State::Store loadedState;
    loadedState.load(path);

    CHECK(loadedState.hasSaveFile());
    CHECK(loadedState.get<std::string>("current-map") == "home-town");
    Tests::checkPoint(
        loadedState.get<SDL_Point>("player-position"),
        SDL_Point{80, 72}
    );
}

TEST_CASE("empty state saves and loads as an empty map") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "empty-state.yaml";
    Engine::State::Store state;

    state.save(path);

    Engine::State::Store loadedState;
    loadedState.load(path);

    CHECK(loadedState.hasSaveFile());
    CHECK(loadedState.empty());
}
