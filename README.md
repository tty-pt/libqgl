<p align="center">
  <img src="logo.png" alt="liblamina logo" width="220" />
</p>

# liblamina

[![C99](https://img.shields.io/badge/C-C99-555?logo=c)](#)
[![BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue)](#)
[![2D Graphics](https://img.shields.io/badge/engine-2D%20graphics-4B8BBE)](#)
[![OpenGL & Framebuffer](https://img.shields.io/badge/backends-OpenGL%20%7C%20GLFW%20%7C%20Linux%20Framebuffer-16A34A)](#)
[![Flexbox UI](https://img.shields.io/badge/layout-flexbox%20UI-blueviolet)](#)

> Ultra-portable 2D immediate-mode rendering, bitmap fonts, tilemaps, and flexbox-style UI layout engine in pure C with OpenGL, GLFW, and direct Linux framebuffer backends. Canonical headers in `<ttypt/lam.h>` and pkg-config package `lamina.pc`.

`liblamina` provides an immediate-mode 2D graphics layer and flexbox-style UI layout engine engineered for small engines, tools, kiosks, and 2D games. It provides a unified immediate-mode API across desktop environments (OpenGL via GLFW on Linux X11/Wayland, macOS, Windows) and direct hardware on embedded Linux (via memory-mapped `/dev/fb0` without X11 or Wayland).

---

## Why liblamina? (2D Graphics & UI Without the Bloat)

Most 2D graphics libraries either drag in massive runtime dependencies (SDL2, Raylib, heavyweight windowing systems) or lock developers into high-level GUI toolkits (GTK, Qt, Electron) that are impossible to run on headless servers, embedded Linux, or bare framebuffer kiosks (`/dev/fb0`).

`liblamina` provides an alternative path in pure C:
- **Zero Heavy Frameworks**: Compact immediate-mode rendering pipeline with procedural pixel hooks (`lam_render`), alpha blending, and double-buffered frame presentation (`lam_flush`).
- **Flexbox UI Built In**: True CSS-style flexbox container trees (`lam_ui_div`), direction/justify/align layout calculations, computed sizing, and caching (`lam-ui.h`) powered by [libcorm](https://github.com/tty-pt/libcorm).
- **Run on Raw Hardware**: Seamlessly falls back to `/dev/fb0` when no display server is present, auto-detecting screen geometry and bit depth via `ioctl(fb_fd, FBIOGET_VSCREENINFO, ...)`.
- **Ecosystem Integration**: Direct integration with [libcorm](https://github.com/tty-pt/libcorm) for UI state caching and [libqsys](https://github.com/tty-pt/libqsys) for cross-platform system primitives.

### The Four Jobs It Does
1. **The 2D Immediate-Mode Canvas**: Primitives, rectangle fills, pixel-by-pixel procedural shaders (`lam_render`), color tinting, and frame presentation via `lam_flush`.
2. **The Sprite & Tilemap Engine**: Load PNG textures, draw whole images or sub-rectangles, sample/paint pixels, and subdivide sprite sheets into grid tilemaps for retro and 2D games (`lam-tm.h`).
3. **The Flexbox UI & Layout Engine**: CSS-like flexbox container trees (`lam_ui_div`), direction/justify/align layout, computed sizing, and caching (`lam-ui.h`) powered by `libcorm`.
4. **The Direct Hardware Fallback**: Runs on standard Linux without X11 or Wayland running—auto-detecting resolution and bit depth on `/dev/fb0` with standard keyboard and gamepad event polling (`lam-key.h`).

---

## Architecture & Subsystems

```
┌────────────────────────────────────────────────────────┐
│                   Application Code                     │
└───────────────┬────────────────────────┬───────────────┘
                │                        │
┌───────────────▼──────────────┐  ┌──────▼───────────────┐
│     Immediate-Mode 2D        │  │   Flexbox UI Engine  │
│  Prims, Textures, Tilemaps   │  │   (lam_ui_div, styles)│
└───────────────┬──────────────┘  └──────┬───────────────┘
                │                        │
┌───────────────▼────────────────────────▼───────────────┐
│                  liblamina Core API                    │
│    Textures, Fonts, Input Handling, Event Dispatch     │
└───────────────┬────────────────────────┬───────────────┘
                │                        │
┌───────────────▼──────────────┐  ┌──────▼───────────────┐
│     Desktop Display Driver   │  │ Embedded Display     │
│        OpenGL / GLFW         │  │ Direct Linux /dev/fb0│
└──────────────────────────────┘  └──────────────────────┘
```

---

## Contents

- [Why liblamina? (2D Graphics & UI Without the Bloat)](#why-liblamina-2d-graphics--ui-without-the-bloat)
- [Architecture & Subsystems](#architecture--subsystems)
- [Quickstart](#quickstart)
- [API Overview & Headers](#api-overview--headers)
- [Display Backends](#display-backends)
  - [Linux Framebuffer Setup (/dev/fb0)](#linux-framebuffer-setup-devfb0)
- [Examples](#examples)
- [Testing & Test Suite](#testing--test-suite)
- [Install & Build from Source](#install--build-from-source)
- [License](#license)

---

## Quickstart

Initialize the display, obtain window dimensions, fill a rectangle, and present the frame:

```c
#include <ttypt/lam.h>

int main(void)
{
	uint32_t w, h;

	lam_init();
	lam_size(&w, &h);
	lam_fill(0, 0, w, h, 0xFF202020); /* Dark gray background */
	lam_flush();
	return 0;
}
```

### Compiling and Linking

`liblamina` links via `-llamina` using the `lamina` pkg-config definition:

```bash
cc my_app.c $(pkg-config --cflags --libs lamina)
```

Or manually:

```bash
cc my_app.c -I/usr/local/include -L/usr/local/lib -llamina -lcorm -lqsys -lpng -lxxhash
```

---

## API Overview & Headers

The API contract lives across five modular headers in `<ttypt/>`:

### 1. Core Rendering & Textures (`<ttypt/lam.h>`)
- **Lifecycle & Frame Presentation**: `lam_init()`, `lam_size(&w, &h)`, `lam_flush()`, `lam_poll()`.
- **Primitives & Fills**: `lam_fill(x, y, w, h, color)`, `lam_tint(color)`.
- **Procedural Shading**: `lam_render(x, y, w, h, shader_fn, ctx)` for custom pixel-by-pixel generators.
- **Texture Operations**: `lam_tex_load(filename)`, `lam_tex_draw(tex, x, y)`, `lam_tex_draw_x(tex, src, dst)`, `lam_tex_pick(tex, x, y)`, `lam_tex_paint(tex, x, y, color)`.

### 2. Flexbox UI Layout & Styling (`<ttypt/lam-ui.h>`)
- **Flexbox Tree**: Construct container hierarchies using `lam_ui_new` (`lam_ui_div_t`) with CSS-equivalent semantics:
  - `direction`: Row, Column, RowReverse, ColumnReverse (`LAM_UI_ROW`, `LAM_UI_COLUMN`).
  - `justify`: FlexStart, Center, FlexEnd, SpaceBetween, SpaceAround (`LAM_UI_JUSTIFY_*`).
  - `align`: FlexStart, Center, FlexEnd, Stretch, Auto (`LAM_UI_ALIGN_*`).
- **Styling**: `lam_ui_stylesheet_init()` and `lam_ui_stylesheet_add()` to configure background colors, borders, margins, padding, and drop shadows (`lam_box_shadow`, `lam_border_radius`).
- **Caching Engine**: Render cache powered by [libcorm](https://github.com/tty-pt/libcorm) for high-performance layout reuse.

### 3. Pre-Rendered Bitmap Fonts (`<ttypt/lam-font.h>`)
- **Font Atlas Loading**: `lam_font_open(path)` loads pre-rendered bitmap font atlases (e.g. converted from BDF files via `bdf2tm.py`).
- **Text Rendering**: `lam_font_draw(...)` renders text at arbitrary integer scales with automatic word wrapping and custom color tinting.
- **Measurement**: `lam_font_measure(...)` calculates bounding boxes before drawing.

### 4. Sprite Tilemaps (`<ttypt/lam-tm.h>`)
- **Atlas Subdivision**: `lam_tm_new(tex, tile_w, tile_h)` segments a spritesheet into a 2D tile grid.
- **Tile Presentation**: Draw individual tiles by index (`lam_tile_draw(tm, tile_id, x, y, w, h, rx, ry)`), perfect for 2D retro games, map renderers, and level editors.

### 5. Input Mapping & Gamepads (`<ttypt/lam-key.h>`)
- **Callbacks**: `lam_key_reg(callback)` for non-blocking raw key events.
- **Key Code Translation**: `lam_key_parse(code)` maps hardware keycodes to ASCII/UTF-8 characters.
- **Gamepads & Hats**: Constants for directional pads and buttons (`LAM_HAT_*`, `LAM_GAMEPAD_*`).

---

## Display Backends

- **Linux Desktop**: Automatically uses GLFW when running under an active X11 or Wayland session (`DISPLAY` or `WAYLAND_DISPLAY` set).
- **Embedded Linux Framebuffer**: Falls back to direct `/dev/fb0` rendering when no display server is present, accessing screen buffers without X11.
- **macOS & Windows**: Native GLFW backend with OpenGL acceleration.

### Linux Framebuffer Setup (`/dev/fb0`)

To run directly on the Linux framebuffer without an X11 or Wayland display server:

1. **Device Permissions**: Ensure your user has read/write permissions to the framebuffer and input event devices:
   ```bash
   sudo usermod -aG video,input $USER
   ```
2. **TTY Console**: Switch to a raw virtual console (e.g. `Ctrl+Alt+F3`) or run in an embedded environment (e.g. Raspberry Pi or kiosk).
3. **Auto-Detection**: `liblamina` automatically reads screen resolution, row stride, and bit depth via `ioctl(fb_fd, FBIOGET_VSCREENINFO, ...)` supporting standard 16bpp and 32bpp RGBA/BGRA modes.

---

## Examples

Six complete example programs demonstrate features from raw pixels to styled UI trees:

```sh
# Generate test fixtures and build examples
python3 tests/generate_fixtures.py
make examples

# Run an example
LD_LIBRARY_PATH=./lib ./examples/01_basic_rendering
```

| Example | Source File | Description |
| :--- | :--- | :--- |
| **01 Basic Rendering** | `examples/01_basic_rendering.c` | Window creation, color fills, and double-buffered presentation |
| **02 Textures** | `examples/02_textures.c` | Loading PNG images, drawing sprites, and sub-region blitting |
| **03 Fonts** | `examples/03_fonts.c` | Bitmap font rendering, scaling, measurement, and text wrapping |
| **04 Tilemaps** | `examples/04_tilemaps.c` | Segmenting sprite atlases into grid tilemaps |
| **05 UI Layout** | `examples/05_ui_layout.c` | Constructing flexbox container trees with `lam_ui_div` |
| **06 Advanced UI** | `examples/06_ui_advanced.c` | Advanced UI styling, borders, layout inspection, and caching |

See [examples/README.md](examples/README.md) for detailed walkthroughs.

---

## Testing & Test Suite

The library includes an extensive test suite covering all public subsystems:

```sh
make test-build       # builds all 8 test binaries
make test             # executes unit tests across core, textures, fonts, tilemaps, and UI
```

Test coverage spans:
- Core rendering and viewport clipping
- PNG texture loading and sub-region blitting
- Font atlas parsing and wrapping math
- Tilemap indexing and bounds checking
- Flexbox layout hierarchy calculations and sizing algorithms
- UI styling and `libcorm` layout cache eviction

---

## Install & Build from Source

### Dependencies

- **Core**: `libcorm`, `libqsys`, `libpng`, `libxxhash`
- **Desktop (GLFW backend)**: `libglfw3-dev`, `libglew-dev`, OpenGL
- **Embedded (Framebuffer backend)**: Linux `/dev/fb0` (standard in Linux kernel)

On Debian / Ubuntu:
```bash
sudo apt-get install build-essential pkg-config libglfw3-dev libglew-dev libpng-dev libxxhash-dev
```

### Build Commands

```sh
make                  # builds lib/liblamina.so
make test             # runs the unit test suite
make examples         # builds example programs in examples/
sudo make install     # installs headers to /usr/local/include/ttypt and lib to /usr/local/lib
```

Prebuilt binary packages are also distributed via tty.pt across Linux (APT / Alpine / Arch / Fedora), macOS (Homebrew), Windows (winget), and OpenBSD. Follow the [tty.pt installation guide](https://github.com/tty-pt/ci/blob/main/docs/install.md) using `liblamina` as the package identifier.

---

## License

Mostly BSD 2-Clause License. Copyright (c) 2025–2026, Paulo André Azevedo Quirino. See `LICENSE`. The Linux framebuffer device initialization is derived from [pedroth's linux-framebuffer](https://github.com/pedroth/linux-framebuffer) and remains under the Apache License, Version 2.0.
