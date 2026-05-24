#include "support.hpp"

#include "engine/runtime/game.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>

TEST_CASE("nodes expose direct and callback-backed properties") {
    Tests::TestNode node;

    node.setProperty("count", 7);
    node.setProperty("title", std::string{"hello"});

    CHECK(node.getProperty<int>("count") == 7);
    CHECK(node.getProperty<std::string>("title") == "hello");
    CHECK(node.hasProperty("active"));
    CHECK_THROWS_AS(node.setProperty("missing", 1), std::runtime_error);
    CHECK_THROWS_AS(
        node.setProperty("count", std::string{"no"}),
        std::runtime_error
    );

    Tests::CallbackNode callbackNode;
    callbackNode.setProperty("count", 5);

    CHECK(callbackNode.getProperty<int>("count") == 10);
    CHECK(callbackNode.callbackCount == 1);
}

TEST_CASE("nodes resolve direct and dotted child paths") {
    auto root = std::make_shared<Engine::Nodes::Base>();
    auto child = std::make_shared<Engine::Nodes::Base>();
    auto grandchild = std::make_shared<Tests::TestNode>();

    root->addChild("child", child);
    child->addChild("grandchild", grandchild);

    CHECK(root->getChild("child") == child);
    CHECK(root->getChild("child.grandchild") == grandchild);
    CHECK(root->getChild<Tests::TestNode>("child.grandchild") == grandchild);
    CHECK(root->getChildren().size() == 1);

    CHECK_THROWS_AS(root->getChild("child."), std::runtime_error);
    CHECK_THROWS_AS(root->getChild("missing"), std::runtime_error);
    CHECK_THROWS_AS(
        root->getChild<Tests::CallbackNode>("child"),
        std::runtime_error
    );
}

TEST_CASE("nodes receive game context when attached to the node manager") {
    auto game = std::make_shared<Engine::Game>();
    auto root = std::make_shared<Engine::Nodes::Base>();
    auto child = std::make_shared<Engine::Nodes::Base>();

    CHECK_THROWS_AS(root->getGame(), std::runtime_error);

    root->addChild("child", child);
    game->nodeManager.setRoot(root);

    CHECK(&root->getGame() == game.get());
    CHECK(&child->getGame() == game.get());
}

TEST_CASE("nodes reject invalid child attachments") {
    auto root = std::make_shared<Engine::Nodes::Base>();

    CHECK_THROWS_AS(root->addChild("null", nullptr), std::runtime_error);
    CHECK_THROWS_AS(root->addChild("self", root), std::runtime_error);
}
