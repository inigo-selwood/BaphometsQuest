#include "tileset.hpp"

#include "../../utils/format.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace Engine::Resource {

Tileset::Tileset(const std::string &path) : Tileset(path, load(path)) {}

Tileset::Tileset(const std::string &path, Data data)
    : Base("tileset"), path(path), tileSize(data.tileSize),
      bytes(std::move(data.bytes)),
      tiles(parseTiles(this->bytes, this->tileSize)) {}

Engine::Resource::ID Tileset::key(const std::string &path) {
    return hashKey("Tileset:" + path);
}

Tileset::Data Tileset::load(const std::string &path) {
    std::ifstream file(path, std::ios::binary);

    if(!file) {
        throw std::runtime_error("Failed to load tileset '" + path + "'");
    }

    SDL_Rect tileSize{0, 0, 0, 0};
    std::uint64_t byteCount = 0;

    file.read(reinterpret_cast<char *>(&tileSize), sizeof(tileSize));
    file.read(reinterpret_cast<char *>(&byteCount), sizeof(byteCount));

    if(!file) {
        throw std::runtime_error(
            "Failed to read tileset metadata '" + path + "'"
        );
    }

    std::vector<std::uint8_t> bytes(byteCount);

    if(!bytes.empty()) {
        file.read(
            reinterpret_cast<char *>(bytes.data()),
            static_cast<std::streamsize>(bytes.size())
        );
    }

    if(!file) {
        throw std::runtime_error("Failed to read tileset data '" + path + "'");
    }

    // Tileset files are strict: metadata byte count must consume the stream
    if(file.peek() != std::char_traits<char>::eof()) {
        throw std::runtime_error(
            "Tileset '" + path + "' has a stream length mismatch"
        );
    }

    return Data{tileSize, bytes};
}

std::unordered_map<std::uint16_t, Tile> Tileset::parseTiles(
    const std::vector<std::uint8_t> &bytes,
    SDL_Rect tileSize
) {
    if(bytes.size() % TILE_BYTES != 0) {
        throw std::runtime_error(
            "Tileset byte count does not match tile record size"
        );
    }

    std::unordered_map<std::uint16_t, Tile> tiles;

    // Records are id low, id high, origin tile x, origin tile y, walk mask
    for(std::size_t offset = 0; offset < bytes.size(); offset += TILE_BYTES) {
        Tile tile;
        tile.id = static_cast<std::uint16_t>(
            bytes[offset] | (bytes[offset + 1] << 8)
        );
        tile.origin = SDL_Point{
            bytes[offset + 2] * tileSize.w,
            bytes[offset + 3] * tileSize.h,
        };
        tile.walkMask = static_cast<char>(bytes[offset + 4]);
        tiles.emplace(tile.id, tile);
    }

    return tiles;
}

void Tileset::save() const {
    std::ofstream file(this->path, std::ios::binary);

    if(!file) {
        throw std::runtime_error(
            "Failed to save tileset '" + this->path + "'"
        );
    }

    const std::uint64_t byteCount = this->bytes.size();

    file.write(
        reinterpret_cast<const char *>(&this->tileSize),
        sizeof(this->tileSize)
    );
    file.write(reinterpret_cast<const char *>(&byteCount), sizeof(byteCount));

    if(!this->bytes.empty()) {
        file.write(
            reinterpret_cast<const char *>(this->bytes.data()),
            static_cast<std::streamsize>(this->bytes.size())
        );
    }

    if(!file) {
        throw std::runtime_error(
            "Failed to write tileset '" + this->path + "'"
        );
    }
}

std::string Tileset::describe() const {
    ::YAML::Node name;
    name["type"] = "Tileset";
    name["path"] = Engine::Format::path(this->path);
    name["tile-width"] = this->tileSize.w;
    name["tile-height"] = this->tileSize.h;
    name["bytes"] = this->bytes.size();

    return this->formatDescription(name);
}

} // namespace Engine::Resource
