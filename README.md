# helios::core

Technical core infrastructure for the helios framework.

## Overview

## Features

## Module surface


## Usage

### C++ module

```cpp
import helios.core;
```

### CMake

When used as a subdirectory, link against the exported target:

```cmake
add_subdirectory(path/to/helios-core)
target_link_libraries(your_target PRIVATE helios::core)
```

## Development

### Central tooling (`helios-devtools`)

`helios-core` can load shared quality tooling from `helios-devtools` via CMake `FetchContent`.
The expected entry file is `cmake/HeliosDevtools.cmake` (required).

Configure-time options:

```bash
cmake -S . -B build -G Ninja \
  -DHELIOS_USE_DEVTOOLS=ON \
  -DHELIOS_DEVTOOLS_GIT_REPOSITORY=https://github.com/thorstensuckow/helios-devtools.git \
  -DHELIOS_DEVTOOLS_GIT_TAG=v1.0.0
```

Use a local checkout instead of Git fetch:

```bash
cmake -S . -B build -G Ninja \
  -DHELIOS_USE_DEVTOOLS=ON \
  -DHELIOS_DEVTOOLS_LOCAL_PATH=/absolute/path/to/helios-devtools
```

Build the project:

```bash
cmake -S . -B build
cmake --build build
```

Quick devtools entrypoint from this repository (no environment variables required):

```bash
sh ./run-devtools.sh format
sh ./run-devtools.sh format-fix
sh ./run-devtools.sh tidy
sh ./run-devtools.sh tidy-fix
```

`run-devtools.sh` is a thin wrapper over existing CMake targets and expects an
already configured build directory (default: `cmake-build-debug`).

Benchmarks are controlled with `HELIOS_BUILD_BENCHMARKS` (top-level default: `ON`, dependency default: `OFF`).

Run tests when test discovery is enabled:

```bash
ctest --test-dir build --output-on-failure
```

Run clang-tidy checks:

```bash
cmake --build cmake-build-debug --target tidy
```

Run clang-tidy with autofix:

```bash
cmake --build cmake-build-debug --target tidy-fix
```

Run clang-format checks:

```bash
cmake --build cmake-build-debug --target format
```

Run clang-format with in-place fixes:

```bash
cmake --build cmake-build-debug --target format-fix
```

Target-specific variants are also available:

```bash
cmake --build cmake-build-debug --target tidy-helios_core
cmake --build cmake-build-debug --target format-helios_core
```

Formatting and clang-tidy checks are sourced from shared `helios-devtools` config.

When `HELIOS_USE_DEVTOOLS=ON`, CMake also provides generated quality targets:

- `tidy-helios_core`
- `tidy-fix-helios_core`
- `format-helios_core`
- `format-fix-helios_core`

## Related repositories

- [`helios-ecs`](https://github.com/thorstensuckow/helios-ecs)
- [`helios-engine`](https://github.com/thorstensuckow/helios-engine)
- [`helios-math`](https://github.com/thorstensuckow/helios-math)
- [`helios-opengl`](https://github.com/thorstensuckow/helios-opengl)
- [`helios-glfw`](https://github.com/thorstensuckow/helios-glfw)
