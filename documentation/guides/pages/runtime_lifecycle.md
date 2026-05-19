# Runtime Lifecycle {#runtime_lifecycle}

`Engine::Game` is the runtime service root. It owns the shared resource, signal,
and node managers, plus the active SDL window and renderer.

Startup
-------

`Game::start()` configures logging, loads `resources/configuration/settings.yaml`,
configures the adaptive timer, parses the renderer clear colour, and delegates
SDL startup to `Engine::Lifecycle::start()`.

The lifecycle startup path initializes SDL, image loading, audio, fonts, the
window, the window icon, the renderer, logical sizing, and native resize
handling. Out-of-range settings are clamped with warnings, while missing or
structurally invalid settings still fail loudly.

Run Loop
--------

`Game::run()` applies queued scene changes at frame boundaries, polls SDL
events, dispatches node hooks, clears and presents the renderer, purges expired
resources, and asks `Engine::Timer` how long to delay before the next frame.

Shutdown
--------

On exit, the active tree receives exit hooks, resources are cleared, and SDL
subsystems are stopped in reverse order.
