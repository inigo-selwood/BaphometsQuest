# Baphomet's Quest

## Requirements

Install the project tools and native libraries for your platform, then use the
Task commands in this repository to build, run, format, and generate docs.

### macOS

```bash
brew install \
  cli11 \
  cmake \
  doxygen \
  go-task \
  ninja \
  node \
  sdl2 \
  sdl2_image \
  sdl2_mixer \
  sdl2_ttf \
  spdlog \
  tinyxml2 \
  yaml-cpp
```

### Linux

Debian or Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  doxygen \
  libcli11-dev \
  libsdl2-dev \
  libsdl2-image-dev \
  libsdl2-mixer-dev \
  libsdl2-ttf-dev \
  libspdlog-dev \
  libtinyxml2-dev \
  libyaml-cpp-dev \
  ninja-build \
  nodejs \
  npm
```

Fedora:

```bash
sudo dnf install -y \
  SDL2-devel \
  SDL2_image-devel \
  SDL2_mixer-devel \
  SDL2_ttf-devel \
  cli11-devel \
  cmake \
  doxygen \
  gcc-c++ \
  ninja-build \
  nodejs \
  npm \
  spdlog-devel \
  tinyxml2-devel \
  yaml-cpp-devel
```

Arch:

```bash
sudo pacman -Syu --needed \
  base-devel \
  cli11 \
  cmake \
  doxygen \
  ninja \
  nodejs \
  npm \
  sdl2 \
  sdl2_image \
  sdl2_mixer \
  sdl2_ttf \
  spdlog \
  tinyxml2 \
  yaml-cpp
```

### Windows

Windows is expected to use Visual Studio Build Tools, CMake, Ninja, Node.js,
Doxygen, and vcpkg for native libraries.

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact
winget install --id Kitware.CMake --exact
winget install --id Ninja-build.Ninja --exact
winget install --id OpenJS.NodeJS.LTS --exact
winget install --id Doxygen.Doxygen --exact
winget install --id Microsoft.Vcpkg --exact

vcpkg install `
  cli11 `
  spdlog `
  sdl2 `
  sdl2-image `
  sdl2-mixer `
  sdl2-ttf `
  tinyxml2 `
  yaml-cpp `
  --triplet x64-windows
vcpkg integrate install
```

## Usage

| Command | Description |
| --- | --- |
| `task` | List available tasks |
| `task run` | Build and run the debug game |
| `task build` | Build an optimised release and bundle resources |
| `task format` | Format source and resource files |
| `task docs` | Build and open Doxygen documentation |
| `task clean` | Remove generated build and tooling output |
