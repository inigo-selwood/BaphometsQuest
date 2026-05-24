#include "support.hpp"

#include "engine/resources/types/map_data.hpp"
#include "engine/resources/types/tileset.hpp"

#include <SDL.h>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <stdexcept>

TEST_CASE("tileset loads tile origins and walk masks from Tiled XML") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "tileset.tsx";
    Tests::writeFile(
        path,
        "<tileset tilewidth=\"8\" tileheight=\"8\" tilecount=\"4\" "
        "columns=\"2\">\n"
        "  <tile id=\"1\">\n"
        "    <properties>\n"
        "      <property name=\"walk-mask\" value=\"9\" />\n"
        "    </properties>\n"
        "  </tile>\n"
        "</tileset>\n"
    );

    const Engine::Resource::Tileset tileset{path.string()};

    Tests::checkRect(tileset.tileSize, SDL_Rect{0, 0, 8, 8});
    CHECK(tileset.tiles.size() == 4);
    Tests::checkPoint(tileset.tiles.at(1).origin, SDL_Point{0, 0});
    Tests::checkPoint(tileset.tiles.at(2).origin, SDL_Point{8, 0});
    Tests::checkPoint(tileset.tiles.at(3).origin, SDL_Point{0, 8});
    CHECK(tileset.tiles.at(2).walkMask == static_cast<char>(9));
}

TEST_CASE("tileset rejects malformed Tiled XML") {
    const std::filesystem::path badRootPath =
        Tests::getTestDirectory() / "bad-tileset-root.tsx";
    const std::filesystem::path badTilePath =
        Tests::getTestDirectory() / "bad-tileset-tile.tsx";

    Tests::writeFile(badRootPath, "<map />\n");
    Tests::writeFile(
        badTilePath,
        "<tileset tilewidth=\"8\" tileheight=\"8\" tilecount=\"1\" "
        "columns=\"1\">\n"
        "  <tile id=\"8\" />\n"
        "</tileset>\n"
    );

    CHECK_THROWS_AS(
        Engine::Resource::Tileset{badRootPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::Tileset{badTilePath.string()},
        std::runtime_error
    );
}

TEST_CASE("map data loads finite CSV tile layers and object metadata") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "finite-map.tmx";
    Tests::writeFile(
        path,
        "<map width=\"2\" height=\"2\">\n"
        "  <layer>\n"
        "    <data encoding=\"csv\">1, 2, 3, 4</data>\n"
        "  </layer>\n"
        "  <objectgroup>\n"
        "    <object name=\"door\" class=\"teleport\" x=\"8\" y=\"16\" "
        "width=\"8\" height=\"8\">\n"
        "      <properties>\n"
        "        <property name=\"chunk\" value=\"home\" />\n"
        "        <property name=\"spawn\" value=\"front-door\" />\n"
        "        <property name=\"options\">\n"
        "          <item value=\"talk\" />\n"
        "          <item value=\"look\" />\n"
        "        </property>\n"
        "      </properties>\n"
        "    </object>\n"
        "  </objectgroup>\n"
        "</map>\n"
    );

    const Engine::Resource::MapData mapData{path.string()};

    Tests::checkRect(mapData.size, SDL_Rect{0, 0, 2, 2});
    CHECK(mapData.getTileCount() == 4);
    CHECK(mapData.getTileID(0) == 1);
    CHECK(mapData.getTileID(SDL_Point{1, 1}) == 4);
    CHECK_THROWS_AS(mapData.getTileID(-1), std::runtime_error);
    CHECK_THROWS_AS(mapData.getTileID(SDL_Point{2, 0}), std::runtime_error);

    const std::vector<Engine::Resource::MapObject> objects =
        mapData.getObjectsAt(SDL_Point{9, 17});
    REQUIRE(objects.size() == 1);
    CHECK(objects.front().name == "door");
    CHECK(objects.front().type == "teleport");
    CHECK(objects.front().properties.at("chunk") == "home");
    CHECK(objects.front().properties.at("spawn") == "front-door");
    CHECK(objects.front().properties.at("options") == "[talk, look]");
}

TEST_CASE("map data flattens infinite-map chunks") {
    const std::filesystem::path path =
        Tests::getTestDirectory() / "chunked-map.tmx";
    Tests::writeFile(
        path,
        "<map infinite=\"1\">\n"
        "  <layer>\n"
        "    <data encoding=\"csv\">\n"
        "      <chunk x=\"-1\" y=\"0\" width=\"1\" height=\"1\">7</chunk>\n"
        "      <chunk x=\"0\" y=\"0\" width=\"1\" height=\"1\">8</chunk>\n"
        "    </data>\n"
        "  </layer>\n"
        "</map>\n"
    );

    const Engine::Resource::MapData mapData{path.string()};

    Tests::checkRect(mapData.size, SDL_Rect{-1, 0, 2, 1});
    CHECK(mapData.getTileID(SDL_Point{-1, 0}) == 7);
    CHECK(mapData.getTileID(SDL_Point{0, 0}) == 8);
}

TEST_CASE("map data rejects unsupported layer data") {
    const std::filesystem::path badEncodingPath =
        Tests::getTestDirectory() / "bad-encoding-map.tmx";
    const std::filesystem::path flippedPath =
        Tests::getTestDirectory() / "flipped-map.tmx";
    const std::filesystem::path shortPath =
        Tests::getTestDirectory() / "short-map.tmx";

    Tests::writeFile(
        badEncodingPath,
        "<map width=\"1\" height=\"1\"><layer><data>1</data></layer></map>\n"
    );
    Tests::writeFile(
        flippedPath,
        "<map width=\"1\" height=\"1\"><layer><data "
        "encoding=\"csv\">2147483649</data></layer></map>\n"
    );
    Tests::writeFile(
        shortPath,
        "<map width=\"2\" height=\"1\"><layer><data "
        "encoding=\"csv\">1</data></layer></map>\n"
    );

    CHECK_THROWS_AS(
        Engine::Resource::MapData{badEncodingPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{flippedPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{shortPath.string()},
        std::runtime_error
    );
}
