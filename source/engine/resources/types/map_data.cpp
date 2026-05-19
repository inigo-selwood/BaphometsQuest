#include "map_data.hpp"

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>

namespace Engine::Resource {

namespace {

constexpr std::size_t TILE_ID_BYTES = sizeof(std::uint16_t);

} // namespace

MapData::MapData(const std::string &path) : MapData(path, load(path)) {}

MapData::MapData(const std::string &path, Data data)
    : Base("map-data"), path(path), size(data.size),
      bytes(std::move(data.bytes)) {}

Engine::Resource::ID MapData::key(const std::string &path) {
    return hashKey("MapData:" + path);
}

MapData::Data MapData::load(const std::string &path) {
    std::ifstream file(path, std::ios::binary);

    if(!file) {
        throw std::runtime_error("Failed to load map data '" + path + "'");
    }

    SDL_Rect size{0, 0, 0, 0};
    std::uint64_t byteCount = 0;

    file.read(reinterpret_cast<char *>(&size), sizeof(size));
    file.read(reinterpret_cast<char *>(&byteCount), sizeof(byteCount));

    if(!file) {
        throw std::runtime_error(
            "Failed to read map data metadata '" + path + "'"
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
        throw std::runtime_error("Failed to read map data '" + path + "'");
    }

    // Map files are strict: metadata byte count must consume the whole stream
    if(file.peek() != std::char_traits<char>::eof()) {
        throw std::runtime_error(
            "Map data '" + path + "' has a stream length mismatch"
        );
    }

    // Each map cell stores one little-endian uint16 tile ID
    const std::size_t expectedBytes =
        static_cast<std::size_t>(size.w * size.h) * TILE_ID_BYTES;

    if(bytes.size() != expectedBytes) {
        throw std::runtime_error(
            "Map data byte count does not match map size '" + path + "'"
        );
    }

    return Data{size, bytes};
}

void MapData::save() const {
    std::ofstream file(this->path, std::ios::binary);

    if(!file) {
        throw std::runtime_error(
            "Failed to save map data '" + this->path + "'"
        );
    }

    const std::uint64_t byteCount = this->bytes.size();

    file.write(
        reinterpret_cast<const char *>(&this->size),
        sizeof(this->size)
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
            "Failed to write map data '" + this->path + "'"
        );
    }
}

std::uint16_t MapData::getTileID(int index) const {
    const std::size_t offset = static_cast<std::size_t>(index) * TILE_ID_BYTES;

    if(offset + TILE_ID_BYTES > this->bytes.size()) {
        throw std::runtime_error("Map data tile index is out of range");
    }

    return static_cast<std::uint16_t>(
        this->bytes[offset] | (this->bytes[offset + 1] << 8)
    );
}

std::uint16_t MapData::getTileID(SDL_Point cell) const {
    if(cell.x < 0 || cell.y < 0 || cell.x >= this->size.w
        || cell.y >= this->size.h) {
        throw std::runtime_error("Map data tile cell is out of range");
    }

    return this->getTileID((cell.y * this->size.w) + cell.x);
}

std::size_t MapData::getTileCount() const {
    return static_cast<std::size_t>(this->size.w * this->size.h);
}

std::string MapData::describe() const {
    ::YAML::Node name;
    name["type"] = "MapData";
    name["path"] = this->path;
    name["width"] = this->size.w;
    name["height"] = this->size.h;
    name["bytes"] = this->bytes.size();

    return this->formatDescription(name);
}

} // namespace Engine::Resource
