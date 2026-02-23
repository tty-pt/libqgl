# libqgl
> A simple portability and ease-of-use driven graphics library based on OpenGL.

Started from forking [pedroth's linux-framebuffer](https://github.com/pedroth/linux-framebuffer) into [7ways](https://github.com/tty-pt/7ways).

## Installation
Check out [these instructions](https://github.com/tty-pt/ci/blob/main/docs/install.md#install-ttypt-packages).
And use "libqgl" as the package name.

## Usage
If you have an editor that uses a language server, then you'll get help about the functions in it.

Quick start:
```c
#include <ttypt/qgl.h>

int main(void)
{
	uint32_t w, h;

	qgl_size(&w, &h);
	qgl_fill(0, 0, w, h, 0xFF202020);
	qgl_flush();
	return 0;
}
```

Backend selection:
- Windows/macOS use the GLFW backend.
- Linux uses GLFW when `DISPLAY` is set; otherwise it uses the framebuffer backend if available.

Modules:
- Core rendering + textures: `include/ttypt/qgl.h`
- UI layout + styling: `include/ttypt/qgl-ui.h`
- Bitmap fonts: `include/ttypt/qgl-font.h`
- Tilemaps: `include/ttypt/qgl-tm.h`

Docs:
```sh
man qgl_flush
man qgl.h
man qgl-ui.h
man qgl-font.h
man qgl-tm.h
```

## Testing

QGL includes a comprehensive test suite covering all public APIs:

```sh
# Build and run all tests
make test

# Build tests without running
make test-build
```

The test suite includes 64 test functions across 8 test files:
- Core rendering and initialization
- Texture loading and drawing
- Font rendering and measurement
- Tilemap creation and rendering
- UI layout engine (flexbox-style)
- UI styling and caching
- Integration scenarios

See individual test files in `tests/` for detailed API usage examples.

## Examples

Six example programs demonstrate QGL features:

```sh
# Build all examples
make examples

# Run an example (requires test fixtures)
cd tests && python3 generate_fixtures.py && cd ..
LD_LIBRARY_PATH=./lib ./examples/01_basic_rendering
```

Available examples:
- `01_basic_rendering.c` - Basic QGL initialization and rendering
- `02_textures.c` - Texture loading and drawing
- `03_fonts.c` - Bitmap font rendering and text wrapping
- `04_tilemaps.c` - Tilemap creation and rendering
- `05_ui_layout.c` - Flexbox-style UI layout
- `06_ui_advanced.c` - Advanced UI with styling and caching

See `examples/README.md` for detailed descriptions and build instructions.
