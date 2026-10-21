## 1.2.0

- **Rebrand to liblamina**: Rebranded library from `libqgl` to `liblamina` (`lib/liblamina.so`, `lamina.pc`).
- **API Migration (`lam_*` / `LAM_*`)**: Migrated all public and internal function symbols, constants, and macros from `qgl_*` / `QGL_*` to `lam_*` / `LAM_*`.
- **Header Reorganization**: Canonical headers renamed to `<ttypt/lam.h>`, `<ttypt/lam-ui.h>`, `<ttypt/lam-font.h>`, `<ttypt/lam-tm.h>`, and `<ttypt/lam-key.h>`.
- **UI API Migration (`lam_ui_*`)**: Migrated `qui_*` / `QUI_*` UI layout engine to `lam_ui_*` / `LAM_UI_*`.
- **Flexbox Engine Fixes**: Fixed `compute_base_sizes` dirty state handling (`LAM_UI_AUTO`), enabled explicit width/height inheritance, and implemented proper inline style merging over stylesheet classes.

## 1.1.0

- **Debut Standalone 2D Graphics Engine**: Consolidated cross-platform 2D graphics layer supporting immediate-mode rendering across OpenGL/GLFW and Linux framebuffer.
- **Unified Backends**: Full support for bitmap fonts, tilemaps, PNG sprites, procedural fills, and keyboard input handling.
- **Modern Corm & Qsys Integration**: Linked against `libcorm` and `libqsys`.

## 1.0.2

- **Explicit initialisation**: `gl_init` moved out of a constructor into an explicit `qgl_init()` — applications now call `qgl_init()` once before any rendering or input.
- **Windows/Apple constructor fixes**: explicitly call the `img`/`png`/`tile` constructors and `input_glfw_construct()` where `__attribute__((constructor))` doesn't fire, so the `qgl_input_glfw` function pointers are always initialised.
- **CI**: add a winget target (`deps_winget` / `deps_winget_tty`), remove the `pacman`/`pacman_mingw` targets, and add an explicit `publish_to`.
- **Docs & housekeeping**: clarify build and backend selection, normalize the examples, shorten the 0.1.0 release notes, and ignore local backup/generated files.

## [0.1.0] - 2026-02-23

### Added
- Full test suite (64 tests across 8 files), test fixtures, and a fixture generator script.
- Six example programs and an examples README.
- UI layout inspection accessors (`qui_get_x`, `qui_get_y`, `qui_get_width`, `qui_get_height`, `qui_get_content_width`, `qui_get_content_height`).

### Changed
- Makefile: added targets to build/run tests and examples; updated `.gitignore` for test and example artifacts.

### Fixed
- Font loading: corrected texture-reference validation to avoid treating reference `0` as missing.
- Font rendering: added NULL checks to prevent a NOWRAP-path segfault.

### Known issues
- Block layout (`QUI_DISPLAY_BLOCK`) does not stack child elements vertically.
- Some UI layout tests remain partial due to layout-engine limitations.