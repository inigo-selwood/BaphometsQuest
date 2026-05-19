# Maps And Tilesets {#maps_and_tilesets}

Tile rendering is split across three concepts:

- `Engine::Resource::Tileset` stores tile metadata and tile size
- `Engine::Resource::MapData` stores tile IDs in map coordinates
- `Engine::Nodes::Tilemap` renders map cells through an atlas texture

Tilesets
--------

A tileset parses fixed-size tile records into a lookup table keyed by tile ID.
The tileset is the source of truth for tile size, so tilemaps and map lookups
use that size when converting between pixels and cells.

Map Data
--------

Map data stores width and height in tile cells plus a little-endian `uint16`
tile ID for each cell. Lookups such as `getTileID(SDL_Point cell)` use map
coordinates, not screen coordinates.

Tilemaps
--------

A tilemap owns resource IDs for its atlas texture, tileset, and map data. During
rendering, it skips tiles outside the logical screen area and draws only tile
IDs that exist in the tileset.

`getTileAt(SDL_Point screenPixel)` converts a screen-space pixel into a map cell
using the tilemap position and tileset tile size, then returns the matching tile
definition.
