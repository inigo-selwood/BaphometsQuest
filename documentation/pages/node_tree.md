# Node Tree {#node_tree}

Nodes are the main composition unit. A scene is just a node used as the root of
the active tree, and `Engine::Nodes::Manager` walks that tree to dispatch the
hooks each node has declared.

Nodes own their children with `std::shared_ptr`. When a child is added, the
active game context is propagated through the subtree so nodes can reach shared
services such as resources and signals.

Properties
----------

Nodes expose editable state through declared properties. A property can either
assign directly to a member or call a setter callback that performs side
effects before storing the value.

```cpp
class Example : public Engine::Nodes::Base {
  public:
    Example() {
        this->declareProperty("position", this->position);
        this->declareProperty("path", this->path, [this](const auto &value) {
            this->updatePath(value);
        });
    }

  private:
    SDL_Point position{0, 0};
    std::string path;
};
```

Hooks
-----

Hooks are opt-in. A node declares only the loop events it needs, such as
`Enter`, `Input`, `Process`, or `Render`. The manager walks the active tree and
calls the matching virtual method only when the hook is declared.

```cpp
auto box = std::make_shared<Engine::Nodes::Box>();
scene->addChild("red-box", box);
box->setProperty("colour", SDL_Color{255, 0, 0, 255});
box->setProperty("size", SDL_Rect{8, 8, 16, 16});
```
