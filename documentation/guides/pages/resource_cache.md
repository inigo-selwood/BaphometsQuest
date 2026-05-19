# Resource Cache {#resource_cache}

Resources are loaded through `Engine::Resource::Manager`. Callers receive a
stable `Engine::Resource::ID`, and the manager keeps enough information to
reconstruct the live resource later if it expires.

Each resource type owns its cache policy:

- `key(...)` returns the stable identity used as the resource ID
- `create(...)` constructs the live resource
- `TTL` controls how long an unused live resource can stay cached
- `describe()` returns YAML-formatted details for logs

A negative `TTL`, currently used by music, keeps the live resource cached until
the manager is cleared.

Using Resources
---------------

Nodes normally store only resource IDs. Rendering code resolves the current live
resource with `get<T>()`, which refreshes access time and reconstructs expired
resources when needed.

```cpp
const auto textureID =
    game.resources.load<Engine::Resource::ImageTexture>(
        game.renderer.get(),
        "resources/textures/tileset.png"
    );

const auto &texture =
    game.resources.get<Engine::Resource::ImageTexture>(textureID);
```

Expiry
------

`purgeExpired()` unloads only the live object for expired entries. The ledger
entry and stable ID remain, so the next `get<T>()` can rebuild the resource
through its stored factory.
