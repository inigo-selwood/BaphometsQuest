# Maps And Tilesets {#maps_and_tilesets}

Tile rendering is split across three concepts:

- `Engine::Resource::Tileset` stores tile metadata and tile size
- `Engine::Resource::MapData` stores tile IDs in map coordinates
- `Engine::Nodes::Tilemap` renders map cells through an atlas texture

Tilesets
--------

A tileset loads a Tiled `.tsx` file into a lookup table keyed by engine tile
ID. Tiled local tile IDs are shifted by one so engine tile ID `0` remains the
empty tile value used by map data.

The tileset is the source of truth for tile size, so tilemaps and map lookups
use that size when converting between pixels and cells. Tile-level
`walk-mask` or `walk_mask` properties are read as movement bitmasks and default
to `0` when omitted.

Map Data
--------

Map data loads the first Tiled `.tmx` tile layer using CSV encoding. Finite
maps use the map width and height directly; infinite maps flatten their chunks
into one bounded grid while preserving the chunk origin in map coordinates.

Lookups such as `getTileID(SDL_Point cell)` use map coordinates, not screen
coordinates. Infinite maps can therefore contain negative cell coordinates when
Tiled chunks sit above or left of the origin.

Tiled stores tile flip flags in the high bits of each global tile ID. The
loader strips those bits before storing IDs, but rejects any map that actually
uses flipped or rotated tiles because tilemap rendering does not support those
transforms yet. Compressed layer data is also intentionally unsupported for now.

Tilemaps
--------

A tilemap owns resource IDs for its atlas texture, tileset, and map data. During
rendering, it skips tiles outside the logical screen area and draws only tile
IDs that exist in the tileset.

`getTileAt(SDL_Point localPixel)` converts a tilemap-local pixel into a map
cell using the tileset tile size, then returns the matching tile definition.
Screen input should be translated by the runtime context before tile lookup.

Scene XML can create a tilemap with paths to the atlas texture, Tiled tileset,
and Tiled map:

```xml
<tilemap
  map="resources/maps/chunks/overworld.tmx"
  name="overworld"
  position="[16, 16]"
  texture="resources/textures/tileset.png"
  tileset="resources/maps/tileset.tsx"
/>
```
