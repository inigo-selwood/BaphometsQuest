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
        Tests::getResourcePath("tilesets/basic.tsx");

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

TEST_CASE("tileset rejects invalid metadata and properties") {
    const std::filesystem::path missingAttributePath =
        Tests::getTestDirectory() / "missing-tileset-attribute.tsx";
    const std::filesystem::path invalidSizePath =
        Tests::getTestDirectory() / "invalid-tileset-size.tsx";
    const std::filesystem::path badWalkMaskPath =
        Tests::getTestDirectory() / "bad-tileset-walk-mask.tsx";

    Tests::writeFile(
        missingAttributePath,
        "<tileset tilewidth=\"8\" tileheight=\"8\" tilecount=\"1\" />\n"
    );
    Tests::writeFile(
        invalidSizePath,
        "<tileset tilewidth=\"0\" tileheight=\"8\" tilecount=\"1\" "
        "columns=\"1\" />\n"
    );
    Tests::writeFile(
        badWalkMaskPath,
        "<tileset tilewidth=\"8\" tileheight=\"8\" tilecount=\"1\" "
        "columns=\"1\">\n"
        "  <tile id=\"0\">\n"
        "    <properties>\n"
        "      <property name=\"walk-mask\" value=\"north\" />\n"
        "    </properties>\n"
        "  </tile>\n"
        "</tileset>\n"
    );

    CHECK_THROWS_AS(
        Engine::Resource::Tileset{missingAttributePath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::Tileset{invalidSizePath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::Tileset{badWalkMaskPath.string()},
        std::runtime_error
    );
}

TEST_CASE("map data loads finite CSV tile layers and object metadata") {
    const std::filesystem::path path =
        Tests::getResourcePath("maps/finite.tmx");

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
        Tests::getResourcePath("maps/chunked.tmx");

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

TEST_CASE("map data rejects malformed map structure") {
    const std::filesystem::path badRootPath =
        Tests::getTestDirectory() / "bad-map-root.tmx";
    const std::filesystem::path noLayerPath =
        Tests::getTestDirectory() / "no-layer-map.tmx";
    const std::filesystem::path noDataPath =
        Tests::getTestDirectory() / "no-data-map.tmx";
    const std::filesystem::path compressedPath =
        Tests::getTestDirectory() / "compressed-map.tmx";
    const std::filesystem::path invalidSizePath =
        Tests::getTestDirectory() / "invalid-size-map.tmx";

    Tests::writeFile(badRootPath, "<tileset />\n");
    Tests::writeFile(noLayerPath, "<map width=\"1\" height=\"1\" />\n");
    Tests::writeFile(noDataPath, "<map><layer /></map>\n");
    Tests::writeFile(
        compressedPath,
        "<map width=\"1\" height=\"1\"><layer><data encoding=\"csv\" "
        "compression=\"zlib\">1</data></layer></map>\n"
    );
    Tests::writeFile(
        invalidSizePath,
        "<map width=\"0\" height=\"1\"><layer><data "
        "encoding=\"csv\">1</data></layer></map>\n"
    );

    CHECK_THROWS_AS(
        Engine::Resource::MapData{badRootPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{noLayerPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{noDataPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{compressedPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{invalidSizePath.string()},
        std::runtime_error
    );
}

TEST_CASE("map data rejects malformed object metadata") {
    const std::filesystem::path missingCoordinatePath =
        Tests::getTestDirectory() / "missing-object-coordinate-map.tmx";
    const std::filesystem::path duplicatePropertyPath =
        Tests::getTestDirectory() / "duplicate-object-property-map.tmx";
    const std::filesystem::path namelessPropertyPath =
        Tests::getTestDirectory() / "nameless-object-property-map.tmx";
    const std::filesystem::path listItemPath =
        Tests::getTestDirectory() / "bad-list-object-property-map.tmx";

    Tests::writeFile(
        missingCoordinatePath,
        "<map width=\"1\" height=\"1\"><layer><data "
        "encoding=\"csv\">1</data></layer><objectgroup><object x=\"0\" "
        "/></objectgroup></map>\n"
    );
    Tests::writeFile(
        duplicatePropertyPath,
        "<map width=\"1\" height=\"1\"><layer><data "
        "encoding=\"csv\">1</data></layer><objectgroup><object x=\"0\" "
        "y=\"0\"><properties><property name=\"tag\" value=\"a\" />"
        "<property name=\"tag\" value=\"b\" /></properties></object>"
        "</objectgroup></map>\n"
    );
    Tests::writeFile(
        namelessPropertyPath,
        "<map width=\"1\" height=\"1\"><layer><data "
        "encoding=\"csv\">1</data></layer><objectgroup><object x=\"0\" "
        "y=\"0\"><properties><property value=\"a\" /></properties></object>"
        "</objectgroup></map>\n"
    );
    Tests::writeFile(
        listItemPath,
        "<map width=\"1\" height=\"1\"><layer><data "
        "encoding=\"csv\">1</data></layer><objectgroup><object x=\"0\" "
        "y=\"0\"><properties><property name=\"options\"><item />"
        "</property></properties></object></objectgroup></map>\n"
    );

    CHECK_THROWS_AS(
        Engine::Resource::MapData{missingCoordinatePath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{duplicatePropertyPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{namelessPropertyPath.string()},
        std::runtime_error
    );
    CHECK_THROWS_AS(
        Engine::Resource::MapData{listItemPath.string()},
        std::runtime_error
    );
}
