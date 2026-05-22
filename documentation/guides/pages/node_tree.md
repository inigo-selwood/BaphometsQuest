# Node Tree {#node_tree}

Nodes are the main composition unit. A scene is just a node used as the root of
the active tree, and `Engine::Nodes::Manager` walks that tree to dispatch the
hooks each node has declared.

Nodes own their children with `std::shared_ptr`. When a child is added, the
active game context is propagated through the subtree so nodes can reach shared
services such as resources and signals.

Child names are optional. Named children can be retrieved with `getChild()`;
unnamed children still participate in hook dispatch and rendering.

Properties
----------

Nodes expose editable state through declared properties. A property can either
assign directly to a member or call a setter callback that performs side
effects before storing the value.

```cpp
class Example : public Engine::Nodes::Object {
  public:
    Example() {
        this->declareProperty("path", this->path, [this](const auto &value) {
            this->updatePath(value);
        });
    }

  private:
    std::string path;
};
```

Native nodes that need a `position` property can inherit from
`Engine::Nodes::Object`, which declares the property and stores the shared
`SDL_Point` member. The position is local to the node's parent, not a manually
managed world coordinate.

Rendering
---------

Render hooks receive an `Engine::Render::Canvas`, not the raw SDL renderer.
Nodes draw in local coordinates and let the runtime apply parent offsets,
canvas-layer mode, and viewport origin.

```cpp
void Sprite::render(Engine::Render::Canvas &canvas) {
    canvas.copy(
        image.handle.get(),
        &frame.region,
        SDL_Rect{0, 0, frame.region.w, frame.region.h}
    );
}
```

`CanvasLayer` nodes define whether their subtree renders in screen space or
world space. Screen space ignores the active camera, while world space subtracts
the runtime viewport origin during canvas drawing.

`Camera` nodes inherit from `Object`. The node manager uses the accumulated
local positions of parent objects to find the active camera focus before
rendering, then centres the viewport on that focus. Missing canvas layers fall
back to screen space, and world layers without an active camera render with the
viewport origin at `[0, 0]`.

Hooks
-----

Hooks are opt-in. A node declares only the loop events it needs, such as
`Enter`, `Input`, `Process`, or `Render`. The manager walks the active tree and
calls the matching virtual method only when the hook is declared.

`setup()` runs once before a node enters normal hook dispatch. Use it for
one-time composition such as loading declarative scene XML.

The base `active` property suppresses all hook dispatch for a node and its
children when set to `false`.

```cpp
auto box = std::make_shared<Engine::Nodes::Box>();
scene->addChild("red-box", box);
box->setProperty("colour", SDL_Color{255, 0, 0, 255});
box->setProperty("size", SDL_Rect{0, 0, 16, 16});
```

Native Nodes
------------

Current native nodes cover simple drawing, text, images, music, maps, a
self-rendering menu, and animated sprites. `Menu` and `Sprite` are examples of
nodes that own structured internal data instead of representing every part as a
child node.
