# Maps And Tilesets {#maps_and_tilesets}

Tile rendering is split across three concepts:

- `Engine::Resource::Tileset` stores tile metadata and tile size
- `Engine::Resource::MapData` stores tile IDs in map coordinates
- `Engine::Nodes::Map` owns the shared atlas texture, tileset, and traversal
  queries
- `<chunk>` elements provide positioned map data inside a map

Tilesets
--------

A tileset loads a Tiled `.tsx` file into a lookup table keyed by engine tile
ID. Tiled local tile IDs are shifted by one so engine tile ID `0` remains the
empty tile value used by map data.

The tileset is the source of truth for tile size, so map chunks and map lookups
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
uses flipped or rotated tiles because map chunk rendering does not support those
transforms yet. Compressed layer data is also intentionally unsupported for now.

Map Nodes
---------

`Engine::Nodes::Map` owns the shared atlas texture and tileset so multiple
chunks can render and answer movement queries from the same source data. A map
must contain at least one internal chunk, which catches incomplete scene XML
during loading and node setup.

Movement checks live on the map instead of individual actors. `canMove(from,
to)` treats the `walk-mask` bit order as top, bottom, left, right:

- `0b1000`: the target cell can be entered from the top
- `0b0100`: the target cell can be entered from the bottom
- `0b0010`: the target cell can be entered from the left
- `0b0001`: the target cell can be entered from the right

Walking into an empty or missing target cell is blocked. Walking out of an
empty current cell is allowed, which lets actors escape out-of-bounds or
incomplete test data rather than becoming stuck.

Chunks
------

A chunk is internal map data, not a standalone node. It owns the resource ID for
one Tiled map data file through its `data` attribute. During rendering, the map
draws each chunk using its shared atlas texture and tileset. Chunks skip tiles
outside the logical screen area and draw only tile IDs that exist in the parent
tileset.

The map translates a map-local pixel into child chunk-local pixels and uses the
first matching chunk for lookup. Empty cells, cells outside the loaded map data,
and tile IDs that are missing from the tileset are treated as absent tiles.

Scene XML can create a map chunk with paths to the atlas texture, Tiled tileset,
and Tiled map under a shared map node:

```xml
<map
  name="world"
  texture="resources/textures/tileset.png"
  tileset="resources/maps/tileset.tsx"
>
  <chunk
    data="resources/maps/chunks/overworld.tmx"
    position="[16, 16]"
  />
</map>
```
