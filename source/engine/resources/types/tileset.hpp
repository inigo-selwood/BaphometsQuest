#pragma once

#include "../base.hpp"

#include <SDL.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::Resource {

/** Single tile definition inside a binary tileset */
class Tile {
  public:
    /** Stable tile ID referenced by map data */
    std::uint16_t id = 0;

    /** Source position in atlas pixels */
    SDL_Point origin{0, 0};

    /** Bitmask reserved for movement rules */
    char walkMask = 0;
};

/** Binary tileset metadata and parsed tile lookup table */
class Tileset : public Engine::Resource::Base {
  public:
    explicit Tileset(const std::string &path);

    /** Time a live cached tileset can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{300};

    /** Create a cache-owned tileset resource */
    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<Tileset>(path);
    }

    /** Return the stable cache ID for a tileset path */
    static Engine::Resource::ID key(const std::string &path);

    /** Save the current tileset bytes back to its source path */
    void save() const;

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Source path used for loading and saving */
    const std::string path;

    /** Tile dimensions in atlas pixels */
    const SDL_Rect tileSize{0, 0, 0, 0};

    /** Raw five-byte tile records */
    const std::vector<std::uint8_t> bytes;

    /** Parsed tiles keyed by tile ID */
    const std::unordered_map<std::uint16_t, Tile> tiles;

  private:
    static constexpr std::size_t TILE_BYTES = 5;

    struct Data {
        SDL_Rect tileSize{0, 0, 0, 0};
        std::vector<std::uint8_t> bytes;
    };

    Tileset(const std::string &path, Data data);

    static Data load(const std::string &path);
    static std::unordered_map<std::uint16_t, Tile>
    parseTiles(const std::vector<std::uint8_t> &bytes, SDL_Rect tileSize);
};

} // namespace Engine::Resource
