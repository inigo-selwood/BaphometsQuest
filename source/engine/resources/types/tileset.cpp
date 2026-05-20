#include "tileset.hpp"

#include "../../utils/format.hpp"

#include <tinyxml2.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace Engine::Resource {

namespace {

int requireIntAttribute(
    const tinyxml2::XMLElement &element,
    const std::string &name,
    const std::string &path
) {
    int value = 0;

    if(element.QueryIntAttribute(name.c_str(), &value)
        != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Tileset '" + path + "' requires integer attribute '" + name + "'"
        );
    }

    return value;
}

int getWalkMask(const tinyxml2::XMLElement &tileElement) {
    const tinyxml2::XMLElement *properties =
        tileElement.FirstChildElement("properties");

    if(properties == nullptr) {
        return 0;
    }

    for(const tinyxml2::XMLElement *property =
            properties->FirstChildElement("property");
        property != nullptr;
        property = property->NextSiblingElement("property")) {
        const char *name = property->Attribute("name");

        if(name == nullptr) {
            continue;
        }

        const std::string propertyName{name};

        if(propertyName != "walk-mask" && propertyName != "walk_mask") {
            continue;
        }

        int value = 0;

        if(property->QueryIntAttribute("value", &value)
            != tinyxml2::XML_SUCCESS) {
            throw std::runtime_error(
                "Tileset walk-mask property must be an integer"
            );
        }

        return value;
    }

    return 0;
}

} // namespace

Tileset::Tileset(const std::string &path) : Tileset(path, load(path)) {}

Tileset::Tileset(const std::string &path, Data data)
    : Base("tileset"), path(path), tileSize(data.tileSize),
      tiles(std::move(data.tiles)) {}

Engine::Resource::ID Tileset::key(const std::string &path) {
    return hashKey("Tileset:" + path);
}

Tileset::Data Tileset::load(const std::string &path) {
    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError error = document.LoadFile(path.c_str());

    if(error != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error(
            "Failed to load Tiled tileset '" + path
            + "': " + document.ErrorStr()
        );
    }

    const tinyxml2::XMLElement *root = document.RootElement();

    if(root == nullptr || std::string(root->Name()) != "tileset") {
        throw std::runtime_error(
            "Tiled tileset '" + path + "' root element must be <tileset>"
        );
    }

    const int tileWidth = requireIntAttribute(*root, "tilewidth", path);
    const int tileHeight = requireIntAttribute(*root, "tileheight", path);
    const int tileCount = requireIntAttribute(*root, "tilecount", path);
    const int columns = requireIntAttribute(*root, "columns", path);

    if(tileWidth <= 0 || tileHeight <= 0 || tileCount < 0 || columns <= 0) {
        throw std::runtime_error(
            "Tiled tileset '" + path + "' has invalid tile dimensions"
        );
    }

    std::unordered_map<std::uint16_t, Tile> tiles;

    for(int localID = 0; localID < tileCount; localID++) {
        Tile tile;
        tile.id = static_cast<std::uint16_t>(localID + 1);
        tile.origin = SDL_Point{
            (localID % columns) * tileWidth,
            (localID / columns) * tileHeight,
        };
        tiles.emplace(tile.id, tile);
    }

    for(const tinyxml2::XMLElement *tileElement =
            root->FirstChildElement("tile");
        tileElement != nullptr;
        tileElement = tileElement->NextSiblingElement("tile")) {
        const int localID = requireIntAttribute(*tileElement, "id", path);

        if(localID < 0 || localID >= tileCount) {
            throw std::runtime_error(
                "Tiled tileset '" + path + "' tile id is out of range"
            );
        }

        tiles.at(static_cast<std::uint16_t>(localID + 1)).walkMask =
            static_cast<char>(getWalkMask(*tileElement));
    }

    return Data{
        SDL_Rect{0, 0, tileWidth, tileHeight},
        tiles,
    };
}

std::string Tileset::describe() const {
    ::YAML::Node name;
    name["type"] = "Tileset";
    name["path"] = Engine::Format::path(this->path);
    name["tile-width"] = this->tileSize.w;
    name["tile-height"] = this->tileSize.h;
    name["tiles"] = this->tiles.size();

    return this->formatDescription(name);
}

} // namespace Engine::Resource
