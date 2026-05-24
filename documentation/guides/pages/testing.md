# Testing {#testing}

Unit tests live under `test/unit`. They are built into the `unit_tests`
executable and run through CTest.

Run:

```bash
task test
```

Generate a local coverage report:

```bash
task coverage
```

The text report prints to the terminal. The HTML report is written to
`build/coverage/html`. Coverage currently uses Clang's source coverage tools
through `xcrun`, matching the local macOS toolchain.

The first test layer focuses on behaviour that can run without opening an SDL
window:

- parsing and formatting helpers
- persistent state storage and YAML save/load
- resource cache ID reuse and reconstruction
- scene XML loading, nested nodes, imports, and failure paths

Keep unit tests fast and local. Prefer temporary files under `build/test` for
resource and XML fixtures so generated test data stays out of Git.
