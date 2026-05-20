#pragma once

#include "../../resources/types/image_texture.hpp"
#include "../../resources/types/map_data.hpp"
#include "../../resources/types/tileset.hpp"
#include "object.hpp"

#include <SDL.h>

#include <string>

namespace Engine::Nodes {

/** Render map data using a tileset and texture atlas */
class Tilemap : public Engine::Nodes::Object {
  public:
    Tilemap();

    void render(SDL_Renderer &renderer) override;

    /** Return the tileset tile at a screen-space pixel position */
    Engine::Resource::Tile getTileAt(SDL_Point screenPixel) const;

  private:
    /** Convert screen pixels to map cell coordinates */
    SDL_Point getCellAt(SDL_Point screenPixel) const;

    /** Divide while rounding toward negative infinity for off-map pixels */
    static int divideFloor(int value, int divisor);

    /** Update the cached atlas texture ID */
    void updateTexture(const std::string &texture);

    /** Update the cached tileset ID */
    void updateTileset(const std::string &tileset);

    /** Update the cached map data ID */
    void updateMap(const std::string &map);

    Engine::Resource::ID textureResourceID = 0;
    Engine::Resource::ID tilesetResourceID = 0;
    Engine::Resource::ID mapResourceID = 0;
    std::string texture;
    std::string tileset;
    std::string map;
};

} // namespace Engine::Nodes
