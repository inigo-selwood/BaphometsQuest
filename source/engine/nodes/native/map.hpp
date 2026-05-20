#pragma once

#include "../../resources/types/image_texture.hpp"
#include "../../resources/types/map_data.hpp"
#include "../../resources/types/tileset.hpp"
#include "object.hpp"

#include <SDL.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Engine::Nodes {

/** Group map chunks and answer map-level movement queries */
class Map : public Engine::Nodes::Object {
  public:
    Map();

    /** Validate that this map owns required resources and chunks */
    void setup() override;

    /** Load internal chunk definitions from XML child elements */
    bool loadXmlChildren(const tinyxml2::XMLElement &element) override;

    void render(Engine::Render::Canvas &canvas) override;

    /** Return the tile at a map-local pixel position when present */
    std::optional<Engine::Resource::Tile>
    findTileAt(SDL_Point localPixel) const;

    /** Return true when movement between map-local pixels is permitted */
    bool canMove(SDL_Point fromPixel, SDL_Point toPixel) const;

  private:
    /** Positioned map data owned by the map rather than the node tree */
    struct Chunk {
        Engine::Resource::ID dataResourceID = 0;
        std::string data;
        SDL_Point position{0, 0};
    };

    /** Parse one chunk element into internal map data */
    Chunk parseChunk(const tinyxml2::XMLElement &chunkElement);

    /** Return the tile ID at a chunk-local pixel position when present */
    std::optional<std::uint16_t> findTileIDAt(
        const Chunk &chunk,
        SDL_Point localPixel,
        SDL_Rect tileSize
    ) const;

    /** Convert chunk-local pixels to map cell coordinates */
    static SDL_Point getCellAt(SDL_Point localPixel, SDL_Rect tileSize);

    /** Divide while rounding toward negative infinity for off-map pixels */
    static int divideFloor(int value, int divisor);

    /** Return the walk-mask bit required to enter the target cell */
    static char getEntryMask(SDL_Point fromPixel, SDL_Point toPixel);

    /** Update the cached atlas texture ID */
    void updateTexture(const std::string &texture);

    /** Update the cached tileset ID */
    void updateTileset(const std::string &tileset);

    Engine::Resource::ID textureResourceID = 0;
    Engine::Resource::ID tilesetResourceID = 0;
    std::string texture;
    std::string tileset;
    std::vector<Chunk> chunks;
};

} // namespace Engine::Nodes
