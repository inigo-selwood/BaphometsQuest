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

class Tile {
  public:
    std::uint16_t id = 0;
    SDL_Point origin{0, 0};
    char walkMask = 0;
};

class Tileset : public Engine::Resource::Base {
  public:
    explicit Tileset(const std::string &path);

    static constexpr std::chrono::seconds TTL{300};

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<Tileset>(path);
    }

    static Engine::Resource::ID key(const std::string &path);

    void save() const;

    std::string describe() const override;

    const std::string path;
    const SDL_Rect tileSize{0, 0, 0, 0};
    const std::vector<std::uint8_t> bytes;
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
