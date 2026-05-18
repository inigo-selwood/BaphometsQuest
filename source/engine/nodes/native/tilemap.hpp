#pragma once

#include "../../resources/types/image_texture.hpp"
#include "../../resources/types/map_data.hpp"
#include "../../resources/types/tileset.hpp"
#include "../base.hpp"

#include <SDL.h>

#include <string>

namespace Engine::Nodes {

/** Render map data using a tileset and texture atlas */
class Tilemap : public Engine::Nodes::Base {
  public:
    Tilemap();

    void render(SDL_Renderer &renderer) override;

    Engine::Resource::Tile getTileAt(SDL_Point screenPixel) const;

  private:
    SDL_Point getCellAt(SDL_Point screenPixel) const;
    static int divideFloor(int value, int divisor);
    void updateTexture(const std::string &texture);
    void updateTileset(const std::string &tileset);
    void updateMap(const std::string &map);

    Engine::Resource::ID textureResourceID = 0;
    Engine::Resource::ID tilesetResourceID = 0;
    Engine::Resource::ID mapResourceID = 0;
    SDL_Point position{0, 0};
    std::string texture;
    std::string tileset;
    std::string map;
};

} // namespace Engine::Nodes
