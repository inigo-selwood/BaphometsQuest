#include "tilemap.hpp"

#include "../../runtime/game.hpp"

#include <stdexcept>

namespace Engine::Nodes {

Tilemap::Tilemap() {
    this->declareHook(Engine::Nodes::Hook::Render);
    this->declareProperty("position", this->position);
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
    this->declareProperty("map", this->map, [this](const std::string &value) {
        this->updateMap(value);
    });
}

void Tilemap::render(SDL_Renderer &renderer) {
    if(this->textureResourceID == 0 || this->tilesetResourceID == 0
        || this->mapResourceID == 0) {
        return;
    }

    Engine::Game &game = this->getGame();
    const Engine::Resource::ImageTexture &texture =
        game.resources.get<Engine::Resource::ImageTexture>(
            this->textureResourceID
        );
    const Engine::Resource::Tileset &tileset =
        game.resources.get<Engine::Resource::Tileset>(this->tilesetResourceID);
    const Engine::Resource::MapData &mapData =
        game.resources.get<Engine::Resource::MapData>(this->mapResourceID);
    const SDL_Rect screenSize = game.getScreenSize();

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
            this->position.x + (mapX * tileset.tileSize.w),
            this->position.y + (mapY * tileset.tileSize.h),
            tileset.tileSize.w,
            tileset.tileSize.h,
        };

        if(SDL_HasIntersection(&destination, &screenSize) == SDL_FALSE) {
            continue;
        }

        if(SDL_RenderCopy(
               &renderer,
               texture.handle.get(),
               &source,
               &destination
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to render tilemap node: ") + SDL_GetError()
            );
        }
    }
}

Engine::Resource::Tile Tilemap::getTileAt(SDL_Point screenPixel) const {
    const SDL_Point cell = this->getCellAt(screenPixel);

    const Engine::Resource::Tileset &tileset =
        this->getGame().resources.get<Engine::Resource::Tileset>(
            this->tilesetResourceID
        );
    const Engine::Resource::MapData &mapData =
        this->getGame().resources.get<Engine::Resource::MapData>(
            this->mapResourceID
        );

    const std::uint16_t tileID = mapData.getTileID(cell);
    const auto tile = tileset.tiles.find(tileID);

    if(tile == tileset.tiles.end()) {
        throw std::runtime_error("Tilemap tile ID is not in tileset");
    }

    return tile->second;
}

SDL_Point Tilemap::getCellAt(SDL_Point screenPixel) const {
    if(this->tilesetResourceID == 0) {
        throw std::runtime_error(
            "Tilemap requires a tileset before converting pixels"
        );
    }

    const Engine::Resource::Tileset &tileset =
        this->getGame().resources.get<Engine::Resource::Tileset>(
            this->tilesetResourceID
        );

    if(tileset.tileSize.w <= 0 || tileset.tileSize.h <= 0) {
        throw std::runtime_error(
            "Tilemap requires tileset tile size before converting pixels"
        );
    }

    const int localX = screenPixel.x - this->position.x;
    const int localY = screenPixel.y - this->position.y;

    return SDL_Point{
        divideFloor(localX, tileset.tileSize.w),
        divideFloor(localY, tileset.tileSize.h),
    };
}

int Tilemap::divideFloor(int value, int divisor) {
    int quotient = value / divisor;
    const int remainder = value % divisor;

    if(remainder != 0 && ((remainder < 0) != (divisor < 0))) {
        --quotient;
    }

    return quotient;
}

void Tilemap::updateTexture(const std::string &texture) {
    this->texture = texture;
    this->textureResourceID = 0;

    if(this->texture.empty()) {
        return;
    }

    Engine::Game &game = this->getGame();

    if(game.renderer == nullptr) {
        throw std::runtime_error(
            "Tilemap requires a renderer before loading texture"
        );
    }

    this->textureResourceID =
        game.resources.load<Engine::Resource::ImageTexture>(
            game.renderer.get(),
            this->texture
        );
}

void Tilemap::updateTileset(const std::string &tileset) {
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

void Tilemap::updateMap(const std::string &map) {
    this->map = map;
    this->mapResourceID = 0;

    if(this->map.empty()) {
        return;
    }

    this->mapResourceID =
        this->getGame().resources.load<Engine::Resource::MapData>(this->map);
}

} // namespace Engine::Nodes
