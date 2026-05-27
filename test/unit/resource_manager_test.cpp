#include "support.hpp"

#include "engine/resources/types/xml.hpp"
#include "engine/resources/types/yaml.hpp"
#include "engine/runtime/managers/resource.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

class ExpiringResource : public Engine::Resource::Base {
  public:
    explicit ExpiringResource(std::string value)
        : Base("expiring-test"), value(std::move(value)) {
        constructions++;
    }

    static constexpr std::chrono::seconds TTL{0};
    static inline int constructions = 0;

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &value) {
        return std::make_unique<ExpiringResource>(value);
    }

    static Engine::Resource::ID key(const std::string &value) {
        return hashKey("ExpiringResource:" + value);
    }

    std::string describe() const override {
        ::YAML::Node node;
        node["type"] = this->name;
        node["value"] = this->value;

        return formatDescription(node);
    }

    std::string value;
};

class PersistentResource : public Engine::Resource::Base {
  public:
    explicit PersistentResource(std::string value)
        : Base("persistent-test"), value(std::move(value)) {
        constructions++;
    }

    static constexpr std::chrono::seconds TTL{-1};
    static inline int constructions = 0;

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &value) {
        return std::make_unique<PersistentResource>(value);
    }

    static Engine::Resource::ID key(const std::string &value) {
        return hashKey("PersistentResource:" + value);
    }

    std::string describe() const override {
        ::YAML::Node node;
        node["type"] = this->name;
        node["value"] = this->value;

        return formatDescription(node);
    }

    std::string value;
};

} // namespace

TEST_CASE("resource manager deduplicates YAML loads by stable ID") {
    const std::filesystem::path path =
        Tests::getResourcePath("documents/settings.yaml");

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
        Tests::getResourcePath("documents/settings.yaml");

    Engine::Resource::Manager resources;
    const Engine::Resource::ID id =
        resources.load<Engine::Resource::YAML>(path.string());

    resources.unload(id);

    const Engine::Resource::YAML &yaml =
        resources.get<Engine::Resource::YAML>(id);
    CHECK((*yaml.node)["answer"].as<int>() == 42);
}

TEST_CASE("resource manager unloads resources by type") {
    ExpiringResource::constructions = 0;
    PersistentResource::constructions = 0;

    Engine::Resource::Manager resources;
    const Engine::Resource::ID expiringID =
        resources.load<ExpiringResource>("temporary");
    const Engine::Resource::ID persistentID =
        resources.load<PersistentResource>("sticky");
    const PersistentResource *persistentBefore =
        &resources.get<PersistentResource>(persistentID);

    resources.unloadAll<ExpiringResource>();

    CHECK(ExpiringResource::constructions == 1);
    CHECK(resources.get<ExpiringResource>(expiringID).value == "temporary");
    CHECK(ExpiringResource::constructions == 2);
    CHECK(&resources.get<PersistentResource>(persistentID)
        == persistentBefore);
    CHECK(PersistentResource::constructions == 1);
}

TEST_CASE("resource manager unloads all except requested resource types") {
    ExpiringResource::constructions = 0;
    PersistentResource::constructions = 0;

    Engine::Resource::Manager resources;
    const Engine::Resource::ID expiringID =
        resources.load<ExpiringResource>("temporary");
    const Engine::Resource::ID persistentID =
        resources.load<PersistentResource>("sticky");
    const PersistentResource *persistentBefore =
        &resources.get<PersistentResource>(persistentID);

    resources.unloadAllExcept<PersistentResource>();

    CHECK(resources.get<ExpiringResource>(expiringID).value == "temporary");
    CHECK(ExpiringResource::constructions == 2);
    CHECK(&resources.get<PersistentResource>(persistentID)
        == persistentBefore);
    CHECK(PersistentResource::constructions == 1);
}

TEST_CASE("resource manager purges expired live resources by TTL") {
    ExpiringResource::constructions = 0;
    PersistentResource::constructions = 0;

    Engine::Resource::Manager resources;
    const Engine::Resource::ID expiringID =
        resources.load<ExpiringResource>("temporary");
    const Engine::Resource::ID persistentID =
        resources.load<PersistentResource>("sticky");
    const PersistentResource *persistentBefore =
        &resources.get<PersistentResource>(persistentID);

    resources.purgeExpired();

    CHECK(resources.get<ExpiringResource>(expiringID).value == "temporary");
    CHECK(ExpiringResource::constructions == 2);
    CHECK(&resources.get<PersistentResource>(persistentID)
        == persistentBefore);
    CHECK(PersistentResource::constructions == 1);
}

TEST_CASE("xml resources load and describe parsed documents") {
    const std::filesystem::path path =
        Tests::getResourcePath("documents/resource.xml");

    const Engine::Resource::XML xml{path.string()};

    REQUIRE(xml.document->RootElement() != nullptr);
    CHECK(std::string{xml.document->RootElement()->Name()} == "root");
    CHECK(xml.document->RootElement()->IntAttribute("answer") == 42);
    CHECK(xml.describe().find("type: XML") != std::string::npos);
}

TEST_CASE("resource manager rejects unknown and mismatched IDs") {
    const std::filesystem::path path =
        Tests::getResourcePath("documents/settings.yaml");

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
