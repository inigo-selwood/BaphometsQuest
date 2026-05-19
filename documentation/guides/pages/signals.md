# Signals {#signals}

Signals are node-scoped and managed by `Engine::Signal::Manager`. A signal is
declared against an owning node and a name, with a fixed argument signature.

```cpp
game.signals.declare<int>(owner, "changed");
game.signals.connect<int>(owner, "changed", [](int value) {
    // Respond to the signal
});
game.signals.emit(owner, "changed", 7);
```

Runtime Checks
--------------

The manager checks that the owner exists, the signal name has been declared, and
the argument types match the declared signature. Mismatches throw
`std::runtime_error`, because they represent code contract errors rather than
recoverable data problems.

Scope
-----

Signals are scoped to their owner node. This keeps names local and avoids a
single global signal namespace while still letting nodes communicate through the
shared game services.

Menu Example
------------

The main menu node declares a `selected` signal with a `std::string` payload.
When Return is pressed, it emits the tag for the active option, such as `play`
or `quit`.

```cpp
game.signals.connect<std::string>(menu, "selected", [](std::string option) {
    // Route the selected menu option
});
```
