#pragma once

#include <SDL.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Engine {

/** Cardinal direction used for tile movement checks. */
enum class Direction : std::uint8_t {
    Top = 1 << 0,
    Bottom = 1 << 1,
    Left = 1 << 2,
    Right = 1 << 3,
};

using TileWalkabilityMask = std::uint8_t;

/** One tile definition within a fixed-size tileset. */
struct Tile {
    SDL_Point position{0, 0};
    TileWalkabilityMask walkability = 0;
};

/** Stores fixed-size tile definitions and per-side walkability. */
class Tileset {
  public:
    static constexpr int TILE_SIZE = 8;

    /** Add a tile and return its index. */
    std::size_t
    addTile(const SDL_Point &position, TileWalkabilityMask walkability);

    /** Return whether the indexed tile can be entered from a direction. */
    bool canEnterFrom(std::size_t index, Direction direction) const;

    /**
     * Return a tile by index.
     *
     * @throws std::out_of_range if index is outside the tileset.
     */
    const Tile &getTile(std::size_t index) const;

    /** Return all tile definitions. */
    const std::vector<Tile> &getTiles() const;

  private:
    std::vector<Tile> tiles;
};

} // namespace Engine
