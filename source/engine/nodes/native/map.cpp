#include "map.hpp"

#include "../../runtime/game.hpp"
#include "../../runtime/render/canvas.hpp"
#include "../../utils/parse.hpp"

#include <tinyxml2.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Engine::Nodes {

Map::Map() {
    this->declareHook(Engine::Nodes::Hook::Render);
    this->declareProperty(
        "texture",
        this->texture,
        [this](const std::string &value) { this->updateTexture(value); }
    );
    this->declareProperty(
        "tileset",
        this->tileset,
        [this](const std::string &value) { this->updateTileset(value); }
    );
}

void Map::setup() {
    if(this->textureResourceID == 0) {
        throw std::runtime_error("Map requires an atlas texture");
    }

    if(this->tilesetResourceID == 0) {
        throw std::runtime_error("Map requires a tileset");
    }

    if(this->chunks.empty()) {
        throw std::runtime_error("Map requires at least one chunk");
    }
}

bool Map::loadXmlChildren(const tinyxml2::XMLElement &element) {
    std::vector<Chunk> parsedChunks;

    for(const tinyxml2::XMLElement *chunkElement = element.FirstChildElement();
        chunkElement != nullptr;
        chunkElement = chunkElement->NextSiblingElement()) {
        parsedChunks.push_back(this->parseChunk(*chunkElement));
    }

    if(parsedChunks.empty()) {
        throw std::runtime_error("Map requires at least one chunk child");
    }

    this->chunks = std::move(parsedChunks);

    return true;
}

void Map::render(Engine::Render::Canvas &canvas) {
    if(this->textureResourceID == 0 || this->tilesetResourceID == 0) {
        return;
    }

    Engine::Game &game = this->getGame();
    const Engine::Resource::ImageTexture &texture =
        game.resources.get<Engine::Resource::ImageTexture>(
            this->textureResourceID
        );
    const Engine::Resource::Tileset &tileset =
        game.resources.get<Engine::Resource::Tileset>(this->tilesetResourceID);

    for(const Chunk &chunk : this->chunks) {
        const Engine::Resource::MapData &mapData =
            game.resources.get<Engine::Resource::MapData>(
                chunk.dataResourceID
            );

        const int tileCount = static_cast<int>(mapData.getTileCount());
        for(int index = 0; index < tileCount; ++index) {
            const std::uint16_t tileID = mapData.getTileID(index);
            const auto tile = tileset.tiles.find(tileID);

            if(tile == tileset.tiles.end()) {
                continue;
            }

            const int column = index % mapData.size.w;
            const int row = index / mapData.size.w;
            const int mapX = mapData.size.x + column;
            const int mapY = mapData.size.y + row;
            const SDL_Rect source{
                tile->second.origin.x,
                tile->second.origin.y,
                tileset.tileSize.w,
                tileset.tileSize.h,
            };
            const SDL_Rect destination{
                chunk.position.x + (mapX * tileset.tileSize.w),
                chunk.position.y + (mapY * tileset.tileSize.h),
                tileset.tileSize.w,
                tileset.tileSize.h,
            };

            if(!canvas.isVisible(destination)) {
                continue;
            }

            canvas.copy(texture.handle.get(), &source, destination);
        }
    }
}

std::optional<Engine::Resource::Tile>
Map::findTileAt(SDL_Point localPixel) const {
    if(this->tilesetResourceID == 0) {
        throw std::runtime_error("Map requires a tileset before tile lookup");
    }

    const Engine::Resource::Tileset &tileset =
        this->getGame().resources.get<Engine::Resource::Tileset>(
            this->tilesetResourceID
        );

    for(const Chunk &chunk : this->chunks) {
        const std::optional<std::uint16_t> tileID = this->findTileIDAt(
            chunk,
            SDL_Point{
                localPixel.x - chunk.position.x,
                localPixel.y - chunk.position.y,
            },
            tileset.tileSize
        );

        if(!tileID.has_value()) {
            continue;
        }

        const auto tile = tileset.tiles.find(*tileID);

        if(tile != tileset.tiles.end()) {
            return tile->second;
        }
    }

    return std::nullopt;
}

Map::Chunk Map::parseChunk(const tinyxml2::XMLElement &chunkElement) {
    const std::string elementName = chunkElement.Name();

    if(elementName != "chunk") {
        throw std::runtime_error(
            "Map child element '" + elementName + "' must be <chunk>"
        );
    }

    const char *dataAttribute = chunkElement.Attribute("data");
    const char *positionAttribute = chunkElement.Attribute("position");

    if(dataAttribute == nullptr || std::string{dataAttribute}.empty()) {
        throw std::runtime_error(
            "Map chunk requires a non-empty data attribute"
        );
    }

    if(positionAttribute == nullptr
        || std::string{positionAttribute}.empty()) {
        throw std::runtime_error(
            "Map chunk requires a non-empty position attribute"
        );
    }

    return Chunk{
        this->getGame().resources.load<Engine::Resource::MapData>(
            dataAttribute
        ),
        dataAttribute,
        Engine::Parse::point(positionAttribute),
    };
}

std::optional<std::uint16_t> Map::findTileIDAt(
    const Chunk &chunk,
    SDL_Point localPixel,
    SDL_Rect tileSize
) const {
    if(chunk.dataResourceID == 0) {
        throw std::runtime_error("Map chunk requires map data before lookup");
    }

    const SDL_Point cell = getCellAt(localPixel, tileSize);
    const Engine::Resource::MapData &mapData =
        this->getGame().resources.get<Engine::Resource::MapData>(
            chunk.dataResourceID
        );

    if(cell.x < mapData.size.x || cell.y < mapData.size.y
        || cell.x >= mapData.size.x + mapData.size.w
        || cell.y >= mapData.size.y + mapData.size.h) {
        return std::nullopt;
    }

    const std::uint16_t tileID = mapData.getTileID(cell);

    if(tileID == 0) {
        return std::nullopt;
    }

    return tileID;
}

SDL_Point Map::getCellAt(SDL_Point localPixel, SDL_Rect tileSize) {
    if(tileSize.w <= 0 || tileSize.h <= 0) {
        throw std::runtime_error(
            "Map chunk requires tile size before converting pixels"
        );
    }

    return SDL_Point{
        divideFloor(localPixel.x, tileSize.w),
        divideFloor(localPixel.y, tileSize.h),
    };
}

int Map::divideFloor(int value, int divisor) {
    int quotient = value / divisor;
    const int remainder = value % divisor;

    if(remainder != 0 && ((remainder < 0) != (divisor < 0))) {
        --quotient;
    }

    return quotient;
}

bool Map::canMove(SDL_Point fromPixel, SDL_Point toPixel) const {
    const std::optional<Engine::Resource::Tile> currentTile =
        this->findTileAt(fromPixel);

    if(!currentTile.has_value()) {
        return true;
    }

    const std::optional<Engine::Resource::Tile> targetTile =
        this->findTileAt(toPixel);

    if(!targetTile.has_value()) {
        return false;
    }

    const char entryMask = getEntryMask(fromPixel, toPixel);

    if(entryMask == 0) {
        return true;
    }

    return (targetTile->walkMask & entryMask) != 0;
}

char Map::getEntryMask(SDL_Point fromPixel, SDL_Point toPixel) {
    constexpr char ENTER_FROM_TOP = 0b1000;
    constexpr char ENTER_FROM_BOTTOM = 0b0100;
    constexpr char ENTER_FROM_LEFT = 0b0010;
    constexpr char ENTER_FROM_RIGHT = 0b0001;

    const int deltaX = toPixel.x - fromPixel.x;
    const int deltaY = toPixel.y - fromPixel.y;

    if(deltaX == 0 && deltaY == 0) {
        return 0;
    }

    if(deltaX != 0 && deltaY != 0) {
        throw std::runtime_error("Map movement must be cardinal");
    }

    if(deltaY < 0) {
        return ENTER_FROM_BOTTOM;
    }

    if(deltaY > 0) {
        return ENTER_FROM_TOP;
    }

    if(deltaX < 0) {
        return ENTER_FROM_RIGHT;
    }

    return ENTER_FROM_LEFT;
}

void Map::updateTexture(const std::string &texture) {
    this->texture = texture;
    this->textureResourceID = 0;

    if(this->texture.empty()) {
        return;
    }

    Engine::Game &game = this->getGame();

    if(game.renderer == nullptr) {
        throw std::runtime_error(
            "Map requires a renderer before loading texture"
        );
    }

    this->textureResourceID =
        game.resources.load<Engine::Resource::ImageTexture>(
            game.renderer.get(),
            this->texture
        );
}

void Map::updateTileset(const std::string &tileset) {
    this->tileset = tileset;
    this->tilesetResourceID = 0;

    if(this->tileset.empty()) {
        return;
    }

    this->tilesetResourceID =
        this->getGame().resources.load<Engine::Resource::Tileset>(
            this->tileset
        );
}

} // namespace Engine::Nodes
