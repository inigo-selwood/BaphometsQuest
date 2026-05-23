#include "map_data.hpp"

#include "../../utils/format.hpp"
#include "../../utils/parse.hpp"

#include <tinyxml2.h>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Engine::Resource {

namespace {

// Tiled stores flip flags in the high four bits of each global tile ID
constexpr std::uint32_t GID_MASK = 0x0FFFFFFF;
constexpr std::uint32_t GID_FLAG_MASK = 0xF0000000;

/** Flattened tile data plus its map-cell bounds */
struct LoadedData {
    SDL_Rect size{0, 0, 0, 0};
    std::vector<std::uint16_t> tiles;
};

/** Read a required integer attribute from a Tiled XML element */
int requireIntAttribute(
    const tinyxml2::XMLElement &element,
    const std::string &name,
    const std::string &path
) {
    int value = 0;

    if(element.QueryIntAttribute(name.c_str(), &value)
        != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Map data '" + path + "' requires integer attribute '" + name + "'"
        );
    }

    return value;
}

/** Parse a Tiled global tile ID and reject unsupported flip flags */
std::uint16_t parseTileID(std::string_view text, const std::string &path) {
    text = Engine::Parse::trim(text);

    if(text.empty()) {
        throw std::runtime_error(
            "Map data '" + path + "' has an empty tile ID"
        );
    }

    std::uint32_t gid = 0;
    const auto result =
        std::from_chars(text.data(), text.data() + text.size(), gid);

    if(result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        throw std::runtime_error(
            "Map data '" + path + "' tile ID is not an integer"
        );
    }

    if((gid & GID_FLAG_MASK) != 0) {
        throw std::runtime_error(
            "Map data '" + path + "' uses unsupported flipped tiles"
        );
    }

    gid &= GID_MASK;

    if(gid > std::numeric_limits<std::uint16_t>::max()) {
        throw std::runtime_error(
            "Map data '" + path + "' tile ID is too large"
        );
    }

    return static_cast<std::uint16_t>(gid);
}

/** Parse a CSV tile payload and hard-fail if its shape is unexpected */
std::vector<std::uint16_t> parseCSV(
    const char *text,
    std::size_t expectedTiles,
    const std::string &path
) {
    if(text == nullptr) {
        throw std::runtime_error("Map data '" + path + "' has no CSV content");
    }

    std::string_view csv{text};
    std::vector<std::uint16_t> tiles;
    std::size_t offset = 0;

    while(offset <= csv.size()) {
        const std::size_t separator = csv.find(',', offset);
        const std::size_t end =
            separator == std::string_view::npos ? csv.size() : separator;
        const std::string_view token =
            Engine::Parse::trim(csv.substr(offset, end - offset));

        if(!token.empty()) {
            tiles.push_back(parseTileID(token, path));
        }

        if(separator == std::string_view::npos) {
            break;
        }

        offset = separator + 1;
    }

    if(tiles.size() != expectedTiles) {
        throw std::runtime_error(
            "Map data '" + path + "' tile count does not match map size"
        );
    }

    return tiles;
}

/**
 * Return the supported tile-layer data element
 *
 * Object layers are gameplay metadata and are intentionally left for a
 * separate parser so tile grids stay small and predictable
 */
const tinyxml2::XMLElement *
getDataElement(const tinyxml2::XMLElement &root, const std::string &path) {
    const tinyxml2::XMLElement *layer = root.FirstChildElement("layer");

    if(layer == nullptr) {
        throw std::runtime_error("Map data '" + path + "' requires a layer");
    }

    const tinyxml2::XMLElement *data = layer->FirstChildElement("data");

    if(data == nullptr) {
        throw std::runtime_error(
            "Map data '" + path + "' requires layer data"
        );
    }

    const char *encoding = data->Attribute("encoding");
    const char *compression = data->Attribute("compression");

    if(encoding == nullptr || std::string{encoding} != "csv") {
        throw std::runtime_error(
            "Map data '" + path + "' must use CSV layer encoding"
        );
    }

    if(compression != nullptr) {
        throw std::runtime_error(
            "Map data '" + path + "' must not use compressed layer data"
        );
    }

    return data;
}

/** Flatten Tiled infinite-map chunks into one bounded tile array */
LoadedData
loadChunkedData(const tinyxml2::XMLElement &data, const std::string &path) {
    const tinyxml2::XMLElement *firstChunk = data.FirstChildElement("chunk");

    if(firstChunk == nullptr) {
        throw std::runtime_error("Map data '" + path + "' has no chunks");
    }

    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    bool hasBounds = false;

    // First pass records map-cell bounds before allocating the flat tile array
    for(const tinyxml2::XMLElement *chunk = firstChunk; chunk != nullptr;
        chunk = chunk->NextSiblingElement("chunk")) {
        const int x = requireIntAttribute(*chunk, "x", path);
        const int y = requireIntAttribute(*chunk, "y", path);
        const int width = requireIntAttribute(*chunk, "width", path);
        const int height = requireIntAttribute(*chunk, "height", path);

        if(width <= 0 || height <= 0) {
            throw std::runtime_error(
                "Map data '" + path + "' chunk size must be positive"
            );
        }

        if(!hasBounds) {
            minX = x;
            minY = y;
            maxX = x + width;
            maxY = y + height;
            hasBounds = true;
            continue;
        }

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x + width);
        maxY = std::max(maxY, y + height);
    }

    const int width = maxX - minX;
    const int height = maxY - minY;
    std::vector<std::uint16_t> tiles(static_cast<std::size_t>(width * height));

    // Second pass copies chunks into the flat array using the recorded bounds
    // as the local origin
    for(const tinyxml2::XMLElement *chunk = firstChunk; chunk != nullptr;
        chunk = chunk->NextSiblingElement("chunk")) {
        const int chunkX = requireIntAttribute(*chunk, "x", path);
        const int chunkY = requireIntAttribute(*chunk, "y", path);
        const int chunkWidth = requireIntAttribute(*chunk, "width", path);
        const int chunkHeight = requireIntAttribute(*chunk, "height", path);
        const std::vector<std::uint16_t> chunkTiles = parseCSV(
            chunk->GetText(),
            static_cast<std::size_t>(chunkWidth * chunkHeight),
            path
        );

        for(int row = 0; row < chunkHeight; row++) {
            for(int column = 0; column < chunkWidth; column++) {
                const int destinationX = chunkX - minX + column;
                const int destinationY = chunkY - minY + row;
                tiles[static_cast<std::size_t>(
                    (destinationY * width) + destinationX
                )] =
                    chunkTiles[static_cast<std::size_t>(
                        (row * chunkWidth) + column
                    )];
            }
        }
    }

    return LoadedData{
        SDL_Rect{minX, minY, width, height},
        tiles,
    };
}

/** Load a fixed-size Tiled map into a zero-origin tile array */
LoadedData loadFiniteData(
    const tinyxml2::XMLElement &root,
    const tinyxml2::XMLElement &data,
    const std::string &path
) {
    const int width = requireIntAttribute(root, "width", path);
    const int height = requireIntAttribute(root, "height", path);

    if(width <= 0 || height <= 0) {
        throw std::runtime_error("Map data '" + path + "' size is invalid");
    }

    return LoadedData{
        SDL_Rect{0, 0, width, height},
        parseCSV(
            data.GetText(),
            static_cast<std::size_t>(width * height),
            path
        ),
    };
}

} // namespace

MapData::MapData(const std::string &path) : MapData(path, load(path)) {}

MapData::MapData(const std::string &path, Data data)
    : Base("map-data"), path(path), size(data.size),
      tiles(std::move(data.tiles)) {}

Engine::Resource::ID MapData::key(const std::string &path) {
    return hashKey("MapData:" + path);
}

MapData::Data MapData::load(const std::string &path) {
    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError error = document.LoadFile(path.c_str());

    if(error != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Failed to load Tiled map '" + path + "': " + document.ErrorStr()
        );
    }

    const tinyxml2::XMLElement *root = document.RootElement();

    if(root == nullptr || std::string(root->Name()) != "map") {
        throw std::runtime_error(
            "Tiled map '" + path + "' root element must be <map>"
        );
    }

    const tinyxml2::XMLElement *data = getDataElement(*root, path);
    LoadedData loadedData;

    // Tiled infinite maps store layer data in chunks, while finite maps keep
    // CSV directly under <data>
    if(data->FirstChildElement("chunk") != nullptr) {
        loadedData = loadChunkedData(*data, path);
    } else {
        loadedData = loadFiniteData(*root, *data, path);
    }

    return Data{
        loadedData.size,
        std::move(loadedData.tiles),
    };
}

std::uint16_t MapData::getTileID(int index) const {
    if(index < 0 || static_cast<std::size_t>(index) >= this->tiles.size()) {
        throw std::runtime_error("Map data tile index is out of range");
    }

    return this->tiles[static_cast<std::size_t>(index)];
}

std::uint16_t MapData::getTileID(SDL_Point cell) const {
    if(cell.x < this->size.x || cell.y < this->size.y
        || cell.x >= this->size.x + this->size.w
        || cell.y >= this->size.y + this->size.h) {
        throw std::runtime_error("Map data tile cell is out of range");
    }

    const int localX = cell.x - this->size.x;
    const int localY = cell.y - this->size.y;

    return this->getTileID((localY * this->size.w) + localX);
}

std::size_t MapData::getTileCount() const {
    return this->tiles.size();
}

std::string MapData::describe() const {
    ::YAML::Node name;
    name["type"] = "MapData";
    name["path"] = Engine::Format::path(this->path);
    name["x"] = this->size.x;
    name["y"] = this->size.y;
    name["width"] = this->size.w;
    name["height"] = this->size.h;
    name["tiles"] = this->tiles.size();

    return this->formatDescription(name);
}

} // namespace Engine::Resource
