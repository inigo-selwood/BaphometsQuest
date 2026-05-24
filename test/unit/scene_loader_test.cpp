#include "support.hpp"

#include "engine/runtime/game.hpp"
#include "engine/runtime/scene_loader.hpp"

#include <SDL.h>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

TEST_CASE("scene loader applies attributes and inlines imports") {
    const std::filesystem::path directory = Tests::getTestDirectory();
    const std::filesystem::path importedPath = directory / "imported.xml";
    const std::filesystem::path scenePath = directory / "scene.xml";

    Tests::writeFile(
        importedPath,
        "<template>\n"
        "  <test-node name=\"imported\" title=\"Imported\" />\n"
        "</template>\n"
    );
    Tests::writeFile(
        scenePath,
        "<scene name=\"unit-scene\">\n"
        "  <test-node\n"
        "    name=\"child\"\n"
        "    colour=\"#11223344\"\n"
        "    count=\"7\"\n"
        "    enabled=\"true\"\n"
        "    position=\"[3, 4]\"\n"
        "    region=\"[1, 2, 3, 4]\"\n"
        "    title=\"Hello\">\n"
        "    <test-node name=\"grandchild\" title=\"Nested\" />\n"
        "  </test-node>\n"
        "  <import path=\"imported.xml\" />\n"
        "</scene>\n"
    );

    auto game = std::make_shared<Engine::Game>();
    auto root = std::make_shared<Engine::Nodes::Base>();
    game->nodeManager.setRoot(root);

    Engine::SceneLoader loader{*root};
    loader.registerNode<Tests::TestNode>("test-node");
    loader.load(scenePath.string());

    const std::shared_ptr<Engine::Nodes::Base> child = root->getChild("child");

    CHECK(child->getProperty<std::string>("title") == "Hello");
    CHECK(child->getProperty<int>("count") == 7);
    CHECK(child->getProperty<bool>("enabled"));
    Tests::checkPoint(
        child->getProperty<SDL_Point>("position"),
        SDL_Point{3, 4}
    );
    Tests::checkRect(
        child->getProperty<SDL_Rect>("region"),
        SDL_Rect{1, 2, 3, 4}
    );
    Tests::checkColour(
        child->getProperty<SDL_Color>("colour"),
        SDL_Color{0x11, 0x22, 0x33, 0x44}
    );

    CHECK(
        root->getChild("child.grandchild")->getProperty<std::string>("title")
        == "Nested"
    );
    CHECK(
        root->getChild("imported")->getProperty<std::string>("title")
        == "Imported"
    );
}

TEST_CASE("scene loader rejects invalid XML roots and elements") {
    const std::filesystem::path directory = Tests::getTestDirectory();
    const std::filesystem::path badRootPath = directory / "bad-root.xml";
    const std::filesystem::path unknownNodePath = directory / "bad-node.xml";

    Tests::writeFile(badRootPath, "<not-scene />\n");
    Tests::writeFile(
        unknownNodePath,
        "<scene name=\"unit-scene\">\n"
        "  <unknown name=\"child\" />\n"
        "</scene>\n"
    );

    auto game = std::make_shared<Engine::Game>();
    auto root = std::make_shared<Engine::Nodes::Base>();
    game->nodeManager.setRoot(root);

    Engine::SceneLoader loader{*root};

    CHECK_THROWS_AS(loader.load(badRootPath.string()), std::runtime_error);
    CHECK_THROWS_AS(loader.load(unknownNodePath.string()), std::runtime_error);
}

TEST_CASE("scene loader rejects missing scene names and import cycles") {
    const std::filesystem::path directory = Tests::getTestDirectory();
    const std::filesystem::path missingNamePath =
        directory / "missing-name.xml";
    const std::filesystem::path cycleAPath = directory / "cycle-a.xml";
    const std::filesystem::path cycleBPath = directory / "cycle-b.xml";

    Tests::writeFile(missingNamePath, "<scene />\n");
    Tests::writeFile(
        cycleAPath,
        "<scene name=\"cycle-a\"><import path=\"cycle-b.xml\" /></scene>\n"
    );
    Tests::writeFile(
        cycleBPath,
        "<template><import path=\"cycle-a.xml\" /></template>\n"
    );

    auto game = std::make_shared<Engine::Game>();
    auto root = std::make_shared<Engine::Nodes::Base>();
    game->nodeManager.setRoot(root);

    Engine::SceneLoader loader{*root};

    CHECK_THROWS_AS(loader.load(missingNamePath.string()), std::runtime_error);
    CHECK_THROWS_AS(loader.load(cycleAPath.string()), std::runtime_error);
}
