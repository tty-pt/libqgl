# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

## Previous releases
Older releases (tags): `v0.0.2`, `v0.0.1`. No detailed changelogs were recorded for those tags.

---

## Compatibility Notes

### qmap 0.6.0
- QGL uses qmap exclusively for in-memory hash maps
- Does not use file-backed maps or QM_MIRROR flags
- Fully compatible with qmap 0.6.0 changes
- **Note**: Fixed bug where font loading incorrectly validated qmap references

### Dependencies
- libqmap - Hash map implementation
- libqsys - System utilities
- libpng - PNG image loading
- libxxhash - Fast hashing
- OpenGL/GLFW - Graphics backend
