#pragma once

#include "../base.hpp"

#include <SDL.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Engine::Resource {

class MapData : public Engine::Resource::Base {
  public:
    explicit MapData(const std::string &path);

    static constexpr std::chrono::seconds TTL{300};

    static std::unique_ptr<Engine::Resource::Base>
    create(Engine::Resource::Manager &, const std::string &path) {
        return std::make_unique<MapData>(path);
    }

    static Engine::Resource::ID key(const std::string &path);

    void save() const;
    std::uint16_t getTileID(int index) const;
    std::uint16_t getTileID(SDL_Point cell) const;
    std::size_t getTileCount() const;

    std::string describe() const override;

    const std::string path;
    const SDL_Rect size{0, 0, 0, 0};
    const std::vector<std::uint8_t> bytes;

  private:
    struct Data {
        SDL_Rect size{0, 0, 0, 0};
        std::vector<std::uint8_t> bytes;
    };

    MapData(const std::string &path, Data data);

    static Data load(const std::string &path);
};

} // namespace Engine::Resource
