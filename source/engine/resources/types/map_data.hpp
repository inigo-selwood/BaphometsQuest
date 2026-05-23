#pragma once

#include "../base.hpp"

#include <SDL.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Engine::Resource {

/** Tiled tile-layer grid loaded from a map file */
class MapData : public Engine::Resource::Base {
  public:
    explicit MapData(const std::string &path);

    /** Time a live cached map can remain unused before eviction */
    static constexpr std::chrono::seconds TTL{300};

    /** Create a cache-owned map data resource */
    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<MapData>(path);
    }

    /** Return the stable cache ID for a map data path */
    static Engine::Resource::ID key(const std::string &path);

    /** Return a tile ID by row-major map index */
    std::uint16_t getTileID(int index) const;

    /** Return a tile ID by map cell coordinate */
    std::uint16_t getTileID(SDL_Point cell) const;

    /** Return the number of tile cells in the map */
    std::size_t getTileCount() const;

    /** Return YAML-formatted resource details for logging */
    std::string describe() const override;

    /** Source path used for loading */
    const std::string path;

    /** Map dimensions in tile cells */
    const SDL_Rect size{0, 0, 0, 0};

    /** Row-major tile IDs */
    const std::vector<std::uint16_t> tiles;

  private:
    struct Data {
        SDL_Rect size{0, 0, 0, 0};
        std::vector<std::uint16_t> tiles;
    };

    MapData(const std::string &path, Data data);

    static Data load(const std::string &path);
};

} // namespace Engine::Resource
