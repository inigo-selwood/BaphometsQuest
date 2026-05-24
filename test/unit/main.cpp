#include "engine/nodes/base.hpp"
#include "engine/resources/types/yaml.hpp"
#include "engine/runtime/game.hpp"
#include "engine/runtime/scene_loader.hpp"
#include "engine/runtime/state.hpp"
#include "engine/utils/format.hpp"
#include "engine/utils/parse.hpp"

#include <SDL.h>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

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

std::filesystem::path getTestDirectory() {
    const std::filesystem::path directory = "build/test/unit";
    std::filesystem::create_directories(directory);

    return directory;
}

void writeFile(const std::filesystem::path &path, std::string_view contents) {
    std::ofstream file{path};

    if(!file) {
        throw std::runtime_error(
            "Failed to write test file '" + path.string() + "'"
        );
    }

    file << contents;
}

template <typename Value>
void expectEqual(
    const Value &actual,
    const Value &expected,
    const std::string &
) {
    CHECK(actual == expected);
}

void expectPoint(
    const SDL_Point &actual,
    const SDL_Point &expected,
    const std::string &
) {
    CHECK(actual.x == expected.x);
    CHECK(actual.y == expected.y);
}

void expectRect(
    const SDL_Rect &actual,
    const SDL_Rect &expected,
    const std::string &
) {
    CHECK(actual.x == expected.x);
    CHECK(actual.y == expected.y);
    CHECK(actual.w == expected.w);
    CHECK(actual.h == expected.h);
}

void expectColour(
    const SDL_Color &actual,
    const SDL_Color &expected,
    const std::string &
) {
    CHECK(actual.r == expected.r);
    CHECK(actual.g == expected.g);
    CHECK(actual.b == expected.b);
    CHECK(actual.a == expected.a);
}

} // namespace

TEST_CASE("parse and format helpers") {
    expectEqual(Engine::Parse::boolean("true"), true, "true should parse");
    expectEqual(Engine::Parse::integer(" 42 "), 42, "integer should parse");
    expectPoint(
        Engine::Parse::point("[3, 4]"),
        SDL_Point{3, 4},
        "point should parse bracket form"
    );
    expectPoint(
        Engine::Parse::point("5,6"),
        SDL_Point{5, 6},
        "point should parse compact form"
    );
    expectRect(
        Engine::Parse::rect("[1, 2, 3, 4]"),
        SDL_Rect{1, 2, 3, 4},
        "rect should parse bracket form"
    );
    expectColour(
        Engine::Parse::colour("#11223344"),
        SDL_Color{0x11, 0x22, 0x33, 0x44},
        "colour should parse hex form"
    );

    expectEqual(
        Engine::Format::path("/one/two/three/four.png"),
        std::string{"two/three/four.png"},
        "path formatter should keep the last three tokens"
    );
    expectEqual(
        Engine::Format::value(SDL_Rect{1, 2, 3, 4}),
        std::string{"[1, 2, 3, 4]"},
        "rect formatter should use bracket notation"
    );

    CHECK_THROWS_AS(Engine::Parse::boolean("yes"), std::runtime_error);
    CHECK_THROWS_AS(Engine::Parse::rect("[1, 2]"), std::runtime_error);
}

TEST_CASE("state store") {
    Engine::State::Store state;

    expectEqual(state.getFlag("met-priest"), false, "absent flag is false");
    state.setFlag("met-priest");
    expectEqual(state.getFlag("met-priest"), true, "stored flag is true");

    state.set("coins", 7);
    state.set("current-map", std::string{"home-town"});
    state.set("player-position", SDL_Point{80, 72});

    expectEqual(state.get<int>("coins"), 7, "int state should round-trip");
    expectEqual(
        state.get<std::string>("current-map"),
        std::string{"home-town"},
        "string state should round-trip"
    );
    expectPoint(
        state.get<SDL_Point>("player-position"),
        SDL_Point{80, 72},
        "point state should round-trip"
    );

    expectEqual(
        state.ensure("health", 3),
        3,
        "ensure should create a missing value"
    );

    CHECK_THROWS_AS(state.get<std::string>("coins"), std::runtime_error);
    CHECK_THROWS_AS(state.get<int>("missing"), std::runtime_error);
}

TEST_CASE("state persistence") {
    const std::filesystem::path path = getTestDirectory() / "state.yaml";
    Engine::State::Store state;

    state.set("current-map", std::string{"home-town"});
    state.set("player-position", SDL_Point{80, 72});
    state.save(path);

    Engine::State::Store loadedState;
    loadedState.load(path);

    expectEqual(
        loadedState.hasSaveFile(),
        true,
        "loading state should mark the save file as present"
    );
    expectEqual(
        loadedState.get<std::string>("current-map"),
        std::string{"home-town"},
        "loaded string state should round-trip"
    );
    expectPoint(
        loadedState.get<SDL_Point>("player-position"),
        SDL_Point{80, 72},
        "loaded point state should round-trip"
    );
}

TEST_CASE("resource manager YAML deduplication") {
    const std::filesystem::path path = getTestDirectory() / "settings.yaml";
    writeFile(path, "answer: 42\n");

    Engine::Resource::Manager resources;
    const Engine::Resource::ID firstID =
        resources.load<Engine::Resource::YAML>(path.string());
    const Engine::Resource::ID secondID =
        resources.load<Engine::Resource::YAML>(path.string());

    expectEqual(firstID, secondID, "duplicate YAML loads should share an ID");

    const Engine::Resource::YAML &yaml =
        resources.get<Engine::Resource::YAML>(firstID);
    expectEqual(
        (*yaml.node)["answer"].as<int>(),
        42,
        "YAML resource should parse test document"
    );

    resources.unload(firstID);
    const Engine::Resource::YAML &reloadedYAML =
        resources.get<Engine::Resource::YAML>(firstID);
    expectEqual(
        (*reloadedYAML.node)["answer"].as<int>(),
        42,
        "unloaded YAML should reconstruct on demand"
    );

    resources.clear();
    CHECK_THROWS_AS(
        resources.get<Engine::Resource::YAML>(firstID),
        std::runtime_error
    );
}

TEST_CASE("scene loader attributes and imports") {
    const std::filesystem::path directory = getTestDirectory();
    const std::filesystem::path importedPath = directory / "imported.xml";
    const std::filesystem::path scenePath = directory / "scene.xml";

    writeFile(
        importedPath,
        "<template>\n"
        "  <test-node name=\"imported\" title=\"Imported\" />\n"
        "</template>\n"
    );
    writeFile(
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
    loader.registerNode<TestNode>("test-node");
    loader.load(scenePath.string());

    const std::shared_ptr<Engine::Nodes::Base> child = root->getChild("child");

    expectEqual(
        child->getProperty<std::string>("title"),
        std::string{"Hello"},
        "scene loader should apply string attributes"
    );
    expectEqual(
        child->getProperty<int>("count"),
        7,
        "scene loader should apply int attributes"
    );
    expectEqual(
        child->getProperty<bool>("enabled"),
        true,
        "scene loader should apply bool attributes"
    );
    expectPoint(
        child->getProperty<SDL_Point>("position"),
        SDL_Point{3, 4},
        "scene loader should apply point attributes"
    );
    expectRect(
        child->getProperty<SDL_Rect>("region"),
        SDL_Rect{1, 2, 3, 4},
        "scene loader should apply rect attributes"
    );
    expectColour(
        child->getProperty<SDL_Color>("colour"),
        SDL_Color{0x11, 0x22, 0x33, 0x44},
        "scene loader should apply colour attributes"
    );

    expectEqual(
        root->getChild("child.grandchild")->getProperty<std::string>("title"),
        std::string{"Nested"},
        "scene loader should support nested child paths"
    );
    expectEqual(
        root->getChild("imported")->getProperty<std::string>("title"),
        std::string{"Imported"},
        "scene loader should inline imported XML templates"
    );
}

TEST_CASE("scene loader failures") {
    const std::filesystem::path directory = getTestDirectory();
    const std::filesystem::path scenePath = directory / "bad-scene.xml";
    writeFile(
        scenePath,
        "<scene name=\"unit-scene\">\n"
        "  <unknown name=\"child\" />\n"
        "</scene>\n"
    );

    auto game = std::make_shared<Engine::Game>();
    auto root = std::make_shared<Engine::Nodes::Base>();
    game->nodeManager.setRoot(root);

    Engine::SceneLoader loader{*root};

    CHECK_THROWS_AS(loader.load(scenePath.string()), std::runtime_error);
}
