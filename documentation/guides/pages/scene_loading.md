# Scene Loading {#scene_loading}

`Engine::SceneLoader` lets a scene populate child nodes from XML while keeping
scene-specific behaviour in C++. XML describes node structure and property
values; the scene registers which element names map to which node classes.

Scene XML files live beside their scene source under `source/scenes/`. The
distribution step copies them into the build output so runtime paths can stay
stable.

XML Shape
---------

The root element must be `<scene>`. Each child element is a node unless the
parent node consumes its own child XML. The element name selects the registered
node class, the optional `name` attribute becomes the child node name, and every
other attribute is applied as a property.

```xml
<scene>
  <label
    name="title"
    font="resources/fonts/Early GameBoy.ttf"
    font-size="8"
    text="baphomet's quest"
    colour="#ffffffff"
    position="[80, 24]"
    justification="centre"
  />
</scene>
```

Nested XML normally becomes child nodes. Some nodes override that behaviour and
parse their child XML as internal data instead. `Menu` owns `<option>` elements,
`Sprite` owns `<animation>` and `<frame>` elements, and `Map` owns `<chunk>`
elements.

Canvas Layers
-------------

`canvas-layer` elements define coordinate space for their children. A screen
layer draws directly to the logical screen; a world layer applies the active
camera-derived viewport origin.

```xml
<canvas-layer mode="world">
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

  <player
    path="resources/textures/tileset.png"
    position="[80, 72]"
    region="[80, 40, 8, 8]"
    step="8"
  >
    <camera position="[4, 4]" />
  </player>
</canvas-layer>
```

Scenes that do not use a canvas layer render as screen space by default. World
layers without an active camera still render using `[0, 0]` as the viewport
origin, which keeps simple map scenes usable while camera behaviour evolves.

Imports
-------

Use `<import path="..."/>` to include another scene XML file. The imported
file must also have a `<scene>` root, and its children are inserted where the
import appears.

```xml
<scene>
  <import path="shared/background.xml" />

  <label
    name="title"
    text="baphomet's quest"
  />
</scene>
```

Relative import paths resolve from the XML file that declares the import.
Import cycles throw at load time.

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

Node-Owned Child XML
--------------------

Nodes can consume their own nested XML by overriding `loadXmlChildren()`.
Returning `true` tells the loader not to instantiate those child elements as
nodes.

```xml
<menu
  cursor-path="resources/textures/tileset.png"
  cursor-region="[232, 16, 8, 8]"
  font="resources/fonts/04B_03.TTF"
  name="main-menu"
  position="[64, 76]"
  font-size="8"
>
  <option
    tag="play"
    text="Play"
  />

  <option
    tag="quit"
    text="Quit"
  />
</menu>
```
