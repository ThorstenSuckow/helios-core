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

Build the project:

```bash
cmake -S . -B build
cmake --build build
```

Run tests when test discovery is enabled:

```bash
ctest --test-dir build --output-on-failure
```

## Related repositories

- [`helios-ecs`](https://github.com/thorstensuckow/helios-ecs)
- [`helios-engine`](https://github.com/thorstensuckow/helios-engine)
- [`helios-math`](https://github.com/thorstensuckow/helios-math)
- [`helios-opengl`](https://github.com/thorstensuckow/helios-opengl)
- [`helios-glfw`](https://github.com/thorstensuckow/helios-glfw)
