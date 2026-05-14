#include "tileset.hpp"

#include <stdexcept>

namespace Engine {

std::size_t
Tileset::addTile(const SDL_Point &position, TileWalkabilityMask walkability) {
    this->tiles.push_back(Tile{position, walkability});
    return this->tiles.size() - 1;
}

bool Tileset::canEnterFrom(std::size_t index, Direction direction) const {
    return (this->getTile(index).walkability
               & static_cast<TileWalkabilityMask>(direction))
        != 0;
}

const Tile &Tileset::getTile(std::size_t index) const {
    if(index >= this->tiles.size()) {
        throw std::out_of_range("Tile index is outside the tileset.");
    }

    return this->tiles.at(index);
}

const std::vector<Tile> &Tileset::getTiles() const {
    return this->tiles;
}

} // namespace Engine
