#pragma once

#include "../base.hpp"

#include <SDL.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine::Resource {

/** Single tile definition inside a Tiled tileset */
class Tile {
  public:
    /** Stable tile ID referenced by map data */
    std::uint16_t id = 0;

    /** Source position in atlas pixels */
    SDL_Point origin{0, 0};

    /** Bitmask reserved for movement rules */
    char walkMask = 0;
};

/** Tiled tileset metadata and parsed tile lookup table */
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

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Source path used for loading */
    const std::string path;

    /** Tile dimensions in atlas pixels */
    const SDL_Rect tileSize{0, 0, 0, 0};

    /** Parsed tiles keyed by tile ID */
    const std::unordered_map<std::uint16_t, Tile> tiles;

  private:
    struct Data {
        SDL_Rect tileSize{0, 0, 0, 0};
        std::unordered_map<std::uint16_t, Tile> tiles;
    };

    Tileset(const std::string &path, Data data);

    static Data load(const std::string &path);
};

} // namespace Engine::Resource
