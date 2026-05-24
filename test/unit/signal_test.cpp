#include "engine/nodes/base.hpp"
#include "engine/runtime/managers/signal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>

TEST_CASE("signal manager declares, connects, and emits node-scoped signals") {
    Engine::Signal::Manager signals;
    auto owner = std::make_shared<Engine::Nodes::Base>();
    int receivedNumber = 0;
    std::string receivedText;

    signals.declare<int, std::string>(owner, "selected");
    signals.connect<int, std::string>(
        owner,
        "selected",
        [&receivedNumber, &receivedText](int number, std::string text) {
            receivedNumber = number;
            receivedText = text;
        }
    );
    signals.emit(owner, "selected", 7, std::string{"play"});

    CHECK(signals.has(owner, "selected"));
    CHECK(receivedNumber == 7);
    CHECK(receivedText == "play");
}

TEST_CASE("signal manager rejects unknown owners and mismatched signatures") {
    Engine::Signal::Manager signals;
    auto owner = std::make_shared<Engine::Nodes::Base>();

    signals.declare<int>(owner, "selected");

    CHECK_THROWS_AS(
        signals.declare<std::string>(owner, "selected"),
        std::runtime_error
    );
    CHECK_THROWS_AS(
        signals.connect<std::string>(owner, "selected", [](std::string) {}),
        std::runtime_error
    );
    CHECK_THROWS_AS(
        signals.emit(owner, "selected", std::string{"bad"}),
        std::runtime_error
    );
    CHECK_THROWS_AS(signals.emit(owner, "missing", 1), std::runtime_error);
    CHECK_THROWS_AS(signals.declare<int>(nullptr, "null"), std::runtime_error);
}
