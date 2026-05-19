# Scene Loading {#scene_loading}

`Engine::SceneLoader` lets a scene populate child nodes from XML while keeping
scene-specific behaviour in C++. XML describes node structure and property
values; the scene registers which element names map to which node classes.

Scene XML files live beside their scene source under `source/scenes/`. The
distribution step copies them into the build output so runtime paths can stay
stable.

XML Shape
---------

The root element must be `<scene>`. Each child element is a node. The element
name selects the registered node class, the `name` attribute becomes the child
node name, and every other attribute is applied as a property.

```xml
<scene>
  <label
    name="title"
    font="resources/fonts/Early GameBoy.ttf"
    size="8"
    text="baphomet's quest"
    colour="#ffffffff"
    position="[80, 24]"
    justification="centre"
  />
</scene>
```

Nested node elements become children of the node created from their parent XML
element.

Registration
------------

Scenes register default-constructible node classes before loading XML.

```cpp
Engine::SceneLoader loader{*this};
loader.registerNode<Engine::Nodes::Label>("label");
loader.registerNode<Engine::Nodes::Music>("music");
loader.load("source/scenes/main_menu/main_menu.xml");
```

The loader does not accept custom factories. If a node needs scene-specific
data, load it from XML first and then set normal typed properties from the
scene.

Text Properties
---------------

XML attributes are converted through `Engine::Nodes::Base::setPropertyFromText`.
Supported text-backed property types are strings, integers, booleans,
`SDL_Point`, `SDL_Rect`, `SDL_Color`, and label justification values.

Custom Typed Properties
-----------------------

Some values are intentionally not encoded as text. The main menu cursor keeps
its selectable options as a typed vector, so the scene sets them after XML load.

```cpp
cursor->setProperty(
    "options",
    std::vector<Components::Cursor::Option>{
        {"play", {48, 76}},
        {"quit", {48, 92}},
    }
);
```
