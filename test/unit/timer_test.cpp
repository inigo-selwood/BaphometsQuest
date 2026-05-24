#include "engine/runtime/timer.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("timer clamps low target frame rates to the minimum") {
    Engine::Timer timer;

    timer.setTargetFrameRate(1);

    CHECK(timer.getDelayDuration(0) == 83);
    CHECK(timer.getDelayDuration(100) == 0);
}

TEST_CASE("timer returns a delay from the configured frame budget") {
    Engine::Timer timer;

    timer.setTargetFrameRate(24);

    CHECK(timer.getDelayDuration(0) == 41);
    CHECK(timer.getDelayDuration(20) == 21);
    CHECK(timer.getDelayDuration(42) == 0);
}

TEST_CASE("timer adapts downward after slow frames when there is room") {
    Engine::Timer timer;

    timer.setTargetFrameRate(24);
    timer.recordFrameDuration(100);

    CHECK(timer.getDelayDuration(0) == 43);
}

TEST_CASE("timer skips adaptive work when target is close to the minimum") {
    Engine::Timer timer;

    timer.setTargetFrameRate(13);
    timer.recordFrameDuration(200);

    CHECK(timer.getDelayDuration(0) == 76);
}
