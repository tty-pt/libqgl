# libqgl

[![C99](https://img.shields.io/badge/C-C99-555?logo=c)](#)
[![BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue)](#)
[![2D graphics](https://img.shields.io/badge/2D-graphics-4B8BBE)](#)

> Cross-platform 2D graphics layer with OpenGL, GLFW and Linux framebuffer backends.

A lightweight, portability-driven 2D graphics library for small engines and
games. It provides a unified immediate-mode API across OpenGL (via GLFW) and
the Linux framebuffer: rectangles, textures, bitmap fonts, tilemaps, a
CSS-like UI layout engine, and keyboard input — without pulling in full
frameworks or GUI toolkits.

Started from forking [pedroth's linux-framebuffer](https://github.com/pedroth/linux-framebuffer)
into [7ways](https://github.com/tty-pt/7ways).

## Contents

- [Features](#features)
- [Install](#install)
- [Build from source](#build-from-source)
- [Quickstart](#quickstart)
- [API overview](#api-overview)
- [Backends](#backends)
- [Examples](#examples)
- [Testing](#testing)
- [Documentation](#documentation)
- [License](#license)

## Features

- **Immediate-mode rendering** — solid fills, per-pixel procedural regions
  (`qgl_render`), colour tinting, and frame presentation via `qgl_flush`.
- **Textures & sprites** — load PNG images, draw whole or sub-regions, read
  and write individual pixels.
- **Bitmap fonts** — load pre-rendered font atlases (e.g. generated from BDF
  with `bdf2tm.py`), measure and render text with wrapping and tinting.
- **Tilemaps** — subdivide a texture atlas and draw individual tiles for 2D
  games.
- **UI layout & styling** — flexbox-style `qui_div` trees, CSS-like style
  fields, and a rendering cache.
- **Input** — keyboard key callbacks, key-code to character parsing, and
  gamepad/hat constants.
- **Portable backends** — one API across GLFW (Linux X11/Wayland, Windows,
  macOS) and the raw Linux framebuffer.

## Install

Prebuilt packages are distributed from tty.pt for Linux (APT / Alpine / Arch /
Fedora-RHEL), macOS (Homebrew), Windows (winget / MSYS2), and OpenBSD. Follow
the [installation instructions](https://github.com/tty-pt/ci/blob/main/docs/install.md)
and use **libqgl** as the package name.

## Build from source

The library builds with a plain `make` (the shared [`mk` include.mk](https://github.com/tty-pt/mk)):

```sh
make                  # builds lib/libqgl.so
make test             # build and run the in-tree test suite
make examples         # build the example programs
sudo make install     # lib + headers + qgl.pc → $(PREFIX), default /usr/local
```

Link it from your own C code:

```sh
cc my_app.c $(pkg-config --cflags --libs qgl)
```

**Dependencies:** `libcorm`, `libqsys`, `libpng`, `libxxhash`, plus
OpenGL/GLEW/GLFW for the GLFW backend or the Linux framebuffer device for the
framebuffer backend.

Required build tools: `make`, a C99 compiler (`gcc` or `clang`), and
`pkg-config`. On Debian/Ubuntu, a minimal install:

```sh
sudo apt-get install build-essential pkg-config libglfw3-dev libpng-dev libxxhash-dev
```

## Quickstart

```c
#include <ttypt/qgl.h>

int main(void)
{
	uint32_t w, h;

	qgl_init();
	qgl_size(&w, &h);
	qgl_fill(0, 0, w, h, 0xFF202020);
	qgl_flush();
	return 0;
}
```

## API overview

The API is split across five headers in `include/ttypt/`:

- **Core rendering + textures** (`qgl.h`) — `qgl_init`, `qgl_size`,
  `qgl_render`, `qgl_fill`, `qgl_flush`; textures `qgl_tex_load`,
  `qgl_tex_draw` / `qgl_tex_draw_x`, `qgl_tex_size`,
  `qgl_tex_pick` / `qgl_tex_paint`, `qgl_tint`.
- **Input** (`qgl.h` + `qgl-key.h`) — `qgl_poll`, `qgl_key_reg`,
  `qgl_key_default_reg`, `qgl_key_val`, `qgl_key_parse`; `QGL_KEY_*` key and
  `QGL_HAT_*` gamepad constants.
- **UI layout + styling** (`qgl-ui.h`) — `qui_div` trees with flexbox-style
  direction/justify/align layout, `qui_style_open` styling, `qui_cache`
  caching, and layout inspection accessors. Uses `libcorm` internally.
- **Bitmap fonts** (`qgl-font.h`) — `qgl_font_open`, text measurement,
  rendering at scale, wrapping, and tinting.
- **Tilemaps** (`qgl-tm.h`) — `qgl_tm_*` tilemap creation and individual tile
  drawing from an atlas.

If your editor has a language server, you'll get help about the functions from
the header comments directly.

## Backends

- **Windows / macOS** use the GLFW backend (requires GLFW development
  libraries).
- **Linux** uses GLFW when the `DISPLAY` environment variable is set (running
  under X11/Wayland); otherwise it falls back to the framebuffer backend when
  a compatible framebuffer device is available (this may require device
  permissions or running as root).

## Examples

Six example programs demonstrate QGL features:

```sh
# Generate fixtures, build examples, then run one
cd tests && python3 generate_fixtures.py && cd ..
make examples
LD_LIBRARY_PATH=./lib ./examples/01_basic_rendering
```

- `01_basic_rendering.c` — basic QGL initialization and rendering
- `02_textures.c` — texture loading and drawing
- `03_fonts.c` — bitmap font rendering and text wrapping
- `04_tilemaps.c` — tilemap creation and rendering
- `05_ui_layout.c` — flexbox-style UI layout
- `06_ui_advanced.c` — advanced UI with styling and caching

See [examples/README.md](examples/README.md) for detailed descriptions and
build instructions.

## Testing

The library includes a test suite covering all public APIs:

```sh
make test             # build and run all 64 tests across 8 test files
make test-build       # build tests without running
```

Test coverage spans core rendering, texture loading and drawing, font
rendering and measurement, tilemaps, the UI layout engine (flexbox-style),
UI styling and caching, and integration scenarios. Fixtures are generated
with `tests/generate_fixtures.py`; see the individual files in `tests/` for
API usage examples.

## Documentation

The API contract lives in the header files in `include/ttypt/`, which are
Doxygen-annotated for man page generation (`make docs`). If manpages are not
installed on your system, consult the headers or the source comments.
Otherwise: [examples/README.md](examples/README.md) for the examples and
[CHANGELOG.md](CHANGELOG.md) for version history.

## License

Mostly BSD 2-Clause License. Copyright (c) 2025, Paulo André Azevedo Quirino.
See `LICENSE`. The framebuffer device initialization is derived from
[pedroth's linux-framebuffer](https://github.com/pedroth/linux-framebuffer)
and remains under the Apache License, Version 2.0.