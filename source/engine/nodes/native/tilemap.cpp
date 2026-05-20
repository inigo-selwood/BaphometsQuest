#include "tilemap.hpp"

#include "../../runtime/game.hpp"
#include "../../runtime/render/canvas.hpp"

#include <stdexcept>

namespace Engine::Nodes {

Tilemap::Tilemap() {
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
    this->declareProperty("map", this->map, [this](const std::string &value) {
        this->updateMap(value);
    });
}

void Tilemap::render(Engine::Render::Canvas &canvas) {
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
            mapX * tileset.tileSize.w,
            mapY * tileset.tileSize.h,
            tileset.tileSize.w,
            tileset.tileSize.h,
        };

        if(!canvas.isVisible(destination)) {
            continue;
        }

        canvas.copy(texture.handle.get(), &source, destination);
    }
}

Engine::Resource::Tile Tilemap::getTileAt(SDL_Point localPixel) const {
    const SDL_Point cell = this->getCellAt(localPixel);

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

SDL_Point Tilemap::getCellAt(SDL_Point localPixel) const {
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

    return SDL_Point{
        divideFloor(localPixel.x, tileset.tileSize.w),
        divideFloor(localPixel.y, tileset.tileSize.h),
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
