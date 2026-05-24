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

    /** Return gameplay objects at a map-local pixel position */
    std::vector<Engine::Resource::MapObject>
    findObjectsAt(SDL_Point localPixel) const;

    /** Return a named gameplay object from the active map when present */
    std::optional<Engine::Resource::MapObject>
    findObject(const std::string &name) const;

  private:
    /** Positioned map data owned by the map rather than the node tree */
    struct Chunk {
        mutable Engine::Resource::ID dataResourceID = 0;
        std::string data;
        std::string name;
        SDL_Point position{0, 0};
    };

    /** Parse one chunk element into internal map data */
    Chunk parseChunk(const tinyxml2::XMLElement &chunkElement);

    /** Return the chunk map data ID, loading it on first active use */
    Engine::Resource::ID ensureChunkData(const Chunk &chunk) const;

    /** Unload live map data for chunks outside the active map */
    void unloadInactiveChunks();

    /** Return true when a chunk participates in render and lookup */
    bool isChunkActive(const Chunk &chunk) const;

    /** Return true when at least one chunk matches the active map */
    bool hasActiveChunk() const;

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

    /** Update active map selection and release inactive chunk data */
    void updateActiveMap(const std::string &activeMap);

    std::string activeMap;

    std::string texture;
    Engine::Resource::ID textureResourceID = 0;

    std::string tileset;
    Engine::Resource::ID tilesetResourceID = 0;

    std::vector<Chunk> chunks;
};

} // namespace Engine::Nodes
