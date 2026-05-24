#include "support.hpp"

#include "engine/resources/types/xml.hpp"
#include "engine/resources/types/yaml.hpp"
#include "engine/runtime/managers/resource.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <stdexcept>

TEST_CASE("resource manager deduplicates YAML loads by stable ID") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "settings.yaml";
    Tests::writeFile(path, "answer: 42\n");

    Engine::Resource::Manager resources;
    const Engine::Resource::ID firstID =
        resources.load<Engine::Resource::YAML>(path.string());
    const Engine::Resource::ID secondID =
        resources.load<Engine::Resource::YAML>(path.string());

    CHECK(firstID == secondID);

    const Engine::Resource::YAML &yaml =
        resources.get<Engine::Resource::YAML>(firstID);
    CHECK((*yaml.node)["answer"].as<int>() == 42);
}

TEST_CASE("resource manager reconstructs unloaded resources on demand") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "reloadable.yaml";
    Tests::writeFile(path, "answer: 42\n");

    Engine::Resource::Manager resources;
    const Engine::Resource::ID id =
        resources.load<Engine::Resource::YAML>(path.string());

    resources.unload(id);

    const Engine::Resource::YAML &yaml =
        resources.get<Engine::Resource::YAML>(id);
    CHECK((*yaml.node)["answer"].as<int>() == 42);
}

TEST_CASE("resource manager rejects unknown and mismatched IDs") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "typed-resource.yaml";
    Tests::writeFile(path, "answer: 42\n");

    Engine::Resource::Manager resources;
    const Engine::Resource::ID id =
        resources.load<Engine::Resource::YAML>(path.string());

    CHECK_THROWS_AS(
        resources.get<Engine::Resource::XML>(id),
        std::runtime_error
    );

    resources.clear();

    CHECK_THROWS_AS(
        resources.get<Engine::Resource::YAML>(id),
        std::runtime_error
    );
}
