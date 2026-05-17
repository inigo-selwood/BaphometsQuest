# Project Style

This file records local style choices for future LLM/codegen work in this
project. Prefer these conventions over generic C++ defaults when they differ.

## Naming

- Filenames and directories use `snake_case`
- Types use `PascalCase`
- Namespaces use `PascalCase`
- Functions, methods, variables, and members use `camelCase`
- Acronyms in identifiers are treated as words: use `getIsoTimestamp`, not `getISOTimestamp`
- Constants also use `camelCase` unless an external API or macro requires otherwise
- External/library names keep their native casing, such as `SDL_*`, `TTF_*`, `Mix_*`, `CLI`, and `YAML::Node`

## Structure

- Keep engine code under `source/engine`
- Keep resource code under `source/engine/resources`
- Keep concrete resource types under `source/engine/resources/types`
- Prefer namespaces that mirror concepts, such as `Engine::Resource` and `Engine::Signal`
- Do not create broad utility namespaces when a focused namespace is clearer
- Prefer direct public properties only when they are intentionally part of the simple API
- Public resource properties should be initialized in constructors and be `const` when they should not change after load

## Ownership

- Use smart pointers for owned SDL and resource handles
- Resource classes are responsible for their own initialization and cleanup
- Managers own collections and IDs, not the inner lifecycle of each resource
- Custom deleters inside classes should be private, named, and inline when the body is only a line or two
- Keep temporary implementation-only deleters local to the `.cpp` file

## Formatting

- Follow `.clang-format`
- Use Doxygen-style comments for public API
- Comments do not end with full stops
- Keep comments succinct and useful; avoid narrating obvious code
- Prefer ASCII unless the surrounding file already has a clear reason for Unicode
- Do not add decorative separators or large comment banners

## APIs

- Use `SDL_Rect` for size-related resource data even when it is a little heavier than separate integers
- Prefer constructor-loaded resources over separate `init` or `destroy` methods
- Prefer type-safe templated manager methods where they keep call sites clear
- Throw `std::runtime_error` for runtime resource, lifecycle, and signal errors
- Keep class names like `Game`, `Node`, and resource type names stable and descriptive

## Logging

- Use `Logger::start` once from `main`
- Logs live in `{path_of_exe}/.logs`
- Keep only the three most recent log files
- File logs always use `trace`
- CLI logging defaults to `off`
- Use log levels consistently:
  - `info` for things that happen once
  - `debug` for things that happen multiple times, but not every cycle
  - `trace` for things that may happen more than once per cycle

## Git

- Commit concept-sized groups rather than tiny mechanical slices
- Keep commits atomic but not granular
- Prefer dependency-shaped commit order: foundations first, then users of those foundations
- Do not recommit unrelated existing work when making follow-up commits
