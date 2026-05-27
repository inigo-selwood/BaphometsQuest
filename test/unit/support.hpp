#pragma once

#include "engine/nodes/base.hpp"

#include <SDL.h>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Tests {

class TestNode : public Engine::Nodes::Base {
  public:
    TestNode() {
        this->declareProperty("colour", this->colour);
        this->declareProperty("count", this->count);
        this->declareProperty("enabled", this->enabled);
        this->declareProperty("position", this->position);
        this->declareProperty("region", this->region);
        this->declareProperty("title", this->title);
    }

  private:
    bool enabled = false;
    int count = 0;
    SDL_Color colour{0, 0, 0, 255};
    SDL_Point position{0, 0};
    SDL_Rect region{0, 0, 0, 0};
    std::string title;
};

class CallbackNode : public Engine::Nodes::Base {
  public:
    CallbackNode() {
        this->declareProperty("count", this->count, [this](int value) {
            this->count = value * 2;
            this->callbackCount++;
        });
    }

    int callbackCount = 0;

  private:
    int count = 0;
};

inline std::filesystem::path getTestDirectory() {
    const std::filesystem::path directory = "build/test/unit";
    std::filesystem::create_directories(directory);

    return directory;
}

inline std::filesystem::path getResourcePath(const std::filesystem::path &path) {
    std::filesystem::path directory = std::filesystem::current_path();

    while(true) {
        const std::filesystem::path candidate =
            directory / "test" / "resources" / path;

        if(std::filesystem::exists(candidate)) {
            return candidate;
        }

        if(!directory.has_parent_path() || directory == directory.parent_path()) {
            break;
        }

        directory = directory.parent_path();
    }

    throw std::runtime_error(
        "Failed to find test resource '" + path.string() + "'"
    );
}

inline void
writeFile(const std::filesystem::path &path, std::string_view contents) {
    std::ofstream file{path};

    if(!file) {
        throw std::runtime_error(
            "Failed to write test file '" + path.string() + "'"
        );
    }

    file << contents;
}

inline void checkPoint(const SDL_Point &actual, const SDL_Point &expected) {
    CHECK(actual.x == expected.x);
    CHECK(actual.y == expected.y);
}

inline void checkRect(const SDL_Rect &actual, const SDL_Rect &expected) {
    CHECK(actual.x == expected.x);
    CHECK(actual.y == expected.y);
    CHECK(actual.w == expected.w);
    CHECK(actual.h == expected.h);
}

inline void checkColour(const SDL_Color &actual, const SDL_Color &expected) {
    CHECK(actual.r == expected.r);
    CHECK(actual.g == expected.g);
    CHECK(actual.b == expected.b);
    CHECK(actual.a == expected.a);
}

} // namespace Tests
