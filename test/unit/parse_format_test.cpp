#include "support.hpp"

#include "engine/utils/format.hpp"
#include "engine/utils/parse.hpp"

#include <SDL.h>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

TEST_CASE("parse helpers accept expected text forms") {
    CHECK(Engine::Parse::boolean("true"));
    CHECK_FALSE(Engine::Parse::boolean("false"));
    CHECK(Engine::Parse::integer(" 42 ") == 42);

    Tests::checkPoint(Engine::Parse::point("[3, 4]"), SDL_Point{3, 4});
    Tests::checkPoint(Engine::Parse::point("5,6"), SDL_Point{5, 6});
    Tests::checkRect(
        Engine::Parse::rect("[1, 2, 3, 4]"),
        SDL_Rect{1, 2, 3, 4}
    );
    Tests::checkColour(
        Engine::Parse::colour("#11223344"),
        SDL_Color{0x11, 0x22, 0x33, 0x44}
    );
}

TEST_CASE("parse helpers reject malformed text") {
    CHECK_THROWS_AS(Engine::Parse::boolean("yes"), std::runtime_error);
    CHECK_THROWS_AS(Engine::Parse::integer("42x"), std::runtime_error);
    CHECK_THROWS_AS(Engine::Parse::point("[1]"), std::runtime_error);
    CHECK_THROWS_AS(Engine::Parse::rect("[1, 2]"), std::runtime_error);
    CHECK_THROWS_AS(Engine::Parse::colour("11223344"), std::runtime_error);
}

TEST_CASE("format helpers produce compact log values") {
    CHECK(
        Engine::Format::path("/one/two/three/four.png") == "two/three/four.png"
    );
    CHECK(Engine::Format::value(true) == "true");
    CHECK(Engine::Format::value(7) == "7");
    CHECK(Engine::Format::value(std::string{"hello"}) == "'hello'");
    CHECK(Engine::Format::value(SDL_Point{1, 2}) == "[1, 2]");
    CHECK(Engine::Format::value(SDL_Rect{1, 2, 3, 4}) == "[1, 2, 3, 4]");
    CHECK(
        Engine::Format::value(SDL_Color{0x11, 0x22, 0x33, 0x44}) == "#11223344"
    );
    CHECK(
        Engine::Format::propertyValue("texture-path", std::string{"a/b/c/d"})
        == "b/c/d"
    );
}
