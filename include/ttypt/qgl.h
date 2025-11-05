/**
 * @file qgl.h
 * @brief Public header for the QGL rendering and input system.
 *
 * QGL is a lightweight, cross-platform 2D graphics layer providing
 * a unified API across OpenGL, framebuffer, and software backends.
 *
 * It manages window/framebuffer creation, texture and sprite
 * drawing, color and tint management, and basic input event handling.
 *
 * QGL is designed for small engines or games that need a minimal,
 * consistent rendering API without full dependence on large
 * frameworks or GUI toolkits.
 */

#ifndef QGL_H
#define QGL_H

#include <stdint.h>
#include <stddef.h>
#include "./qgl-key.h"

/*───────────────────────────────────────────────*
 *              CONSTANTS & TYPES                *
 *───────────────────────────────────────────────*/

/**
 * @defgroup qgl_types QGL types and constants
 * @brief Fundamental types and constants used by QGL.
 * @{
 */

/** @brief Input field flags used by keyboard handlers. */
enum qgl_key_flags {
	IF_MULTILINE = 1, /**< Allows multi-line input (ENTER adds newlines). */
	IF_NUMERIC   = 2, /**< Restricts input to numeric characters. */
};

/**
 * @brief Software rendering callback.
 *
 * Called once per pixel for procedural or CPU-based rendering.
 *
 * @param[out] color Pointer to the pixel in BGRA format.
 * @param[in]  x     X coordinate of the pixel.
 * @param[in]  y     Y coordinate of the pixel.
 * @param[in]  ctx   Optional user data pointer.
 */
typedef void qgl_lambda_t(uint8_t *color,
                          uint32_t x, uint32_t y,
                          void *ctx);

/**
 * @brief Obtain the current framebuffer dimensions.
 *
 * Retrieves the logical pixel dimensions of the active render target
 * (window, screen, or offscreen buffer).
 *
 * @param[out] width  Framebuffer width in pixels.
 * @param[out] height Framebuffer height in pixels.
 */
void qgl_size(uint32_t *width, uint32_t *height);

/** Default white RGBA tint (no color modulation). */
static const uint32_t qgl_default_tint = 0xFFFFFFFF;

/**
 * @brief Keyboard event callback prototype.
 *
 * @param[in] code  Platform key code.
 * @param[in] type  Event type (press, release, repeat).
 * @param[in] value Event state (1 = pressed, 0 = released).
 * @return Non-zero to consume the event, zero to propagate.
 */
typedef int qgl_key_cb_t(unsigned short code,
                         unsigned short type,
                         int value);

/** @} */


/*───────────────────────────────────────────────*
 *                 RENDERING API                 *
 *───────────────────────────────────────────────*/

/**
 * @defgroup qgl_render QGL rendering
 * @brief Immediate-mode drawing and frame presentation.
 * @{
 */

/**
 * @brief Render a region using a user-defined callback.
 *
 * Executes a callback per pixel, useful for procedural effects
 * or software rendering directly into the framebuffer.
 *
 * @param[in] lambda Per-pixel callback function.
 * @param[in] x,y    Region origin.
 * @param[in] w,h    Region size in pixels.
 * @param[in] ctx    Optional user context.
 */
void qgl_render(qgl_lambda_t *lambda,
                int32_t x, int32_t y,
                uint32_t w, uint32_t h,
                void *ctx);

/**
 * @brief Draw a solid-colored rectangle.
 *
 * Fills a rectangular region with the specified color.
 *
 * @param[in] x,y   Position in pixels.
 * @param[in] w,h   Dimensions.
 * @param[in] color 32-bit RGBA color.
 */
void qgl_fill(int32_t x, int32_t y,
              uint32_t w, uint32_t h,
              uint32_t color);

/**
 * @brief Draw a rectangle with rounded corners and optional border.
 *
 * @param[in] background_color Fill color inside the rounded shape.
 * @param[in] border_color     Color of the border line.
 * @param[in] x,y              Top-left corner.
 * @param[in] w,h              Width and height in pixels.
 * @param[in] tl,tr,br,bl      Radii of each corner.
 * @param[in] border_width     Width of the border line.
 */
void qgl_border_radius(uint32_t background_color,
                       uint32_t border_color,
                       int32_t x, int32_t y,
                       uint32_t w, uint32_t h,
                       float tl, float tr,
                       float br, float bl,
                       float border_width);

/**
 * @brief Present the current framebuffer.
 *
 * Flushes pending draw commands and updates the visible frame
 * through the active backend (OpenGL swap, FB copy, etc.).
 */
void qgl_flush(void);

/** @} */


/*───────────────────────────────────────────────*
 *              TEXTURE / SPRITE API             *
 *───────────────────────────────────────────────*/

/**
 * @defgroup qgl_texture QGL textures and sprites
 * @brief Texture management and sprite drawing.
 * @{
 */

/**
 * @brief Draw a subregion of a texture to screen.
 *
 * @param[in] ref Texture reference ID.
 * @param[in] x,y Destination position.
 * @param[in] cx,cy Source region origin.
 * @param[in] sw,sh Source region size.
 * @param[in] dw,dh Destination dimensions.
 * @param[in] tint RGBA tint multiplier.
 */
void qgl_tex_draw_x(uint32_t ref,
                    int32_t x, int32_t y,
                    uint32_t cx, uint32_t cy,
                    uint32_t sw, uint32_t sh,
                    uint32_t dw, uint32_t dh,
                    uint32_t tint);

/**
 * @brief Draw an entire texture scaled to fit a rectangle.
 *
 * @param[in] ref Texture reference ID.
 * @param[in] x,y Destination position.
 * @param[in] dw,dh Destination dimensions.
 */
void qgl_tex_draw(uint32_t ref,
                  int32_t x, int32_t y,
                  uint32_t dw, uint32_t dh);

/**
 * @brief Load an image file into a texture.
 *
 * The supported formats depend on the build configuration.
 *
 * @param[in] filename Path to the image file.
 * @return Texture reference ID.
 */
unsigned qgl_tex_load(const char *filename);

/**
 * @brief Save a texture to disk (if supported by backend).
 *
 * @param[in] ref Texture reference ID.
 */
void qgl_tex_save(unsigned ref);

/**
 * @brief Get the pixel dimensions of a texture.
 *
 * @param[out] w Width in pixels.
 * @param[out] h Height in pixels.
 * @param[in]  ref Texture reference ID.
 */
void qgl_tex_size(uint32_t *w, uint32_t *h, unsigned ref);

/**
 * @brief Read a pixel color from a texture.
 *
 * @param[in] ref Texture reference ID.
 * @param[in] x,y Pixel coordinates.
 * @return 32-bit BGRA color value.
 */
uint32_t qgl_tex_pick(unsigned ref, uint32_t x, uint32_t y);

/**
 * @brief Write a single pixel into a texture.
 *
 * @param[in] ref Texture reference ID.
 * @param[in] x,y Target pixel coordinates.
 * @param[in] color 32-bit BGRA color.
 */
void qgl_tex_paint(unsigned ref,
                   uint32_t x, uint32_t y,
                   uint32_t color);

/**
 * @brief Apply a global color tint to future draw calls.
 *
 * @param[in] tint RGBA color multiplier.
 */
void qgl_tint(uint32_t tint);

/** @} */


/*───────────────────────────────────────────────*
 *                 INPUT API                     *
 *───────────────────────────────────────────────*/

/**
 * @defgroup qgl_input QGL input handling
 * @brief Keyboard polling and event dispatch.
 * @{
 */

/**
 * @brief Poll and process pending input events.
 *
 * Backend-specific function that updates keyboard state
 * and invokes registered key callbacks.
 */
void qgl_poll(void);

/**
 * @brief Register a callback for a specific key.
 *
 * @param[in] key Key code (platform dependent).
 * @param[in] cb  Callback function.
 */
void qgl_key_reg(unsigned short key, qgl_key_cb_t *cb);

/**
 * @brief Register a global keyboard callback.
 *
 * Called for all key events that aren’t handled by specific bindings.
 *
 * @param[in] cb Callback function.
 */
void qgl_key_default_reg(qgl_key_cb_t *cb);

/**
 * @brief Translate a key code into its character value.
 *
 * @param[in] code  Key code.
 * @return          Character code or 0 if not printable.
 */
unsigned short qgl_key_val(unsigned short code);

/**
 * @brief Append a character based on a key press into a string buffer.
 *
 * @param[out] target Output text buffer.
 * @param[in]  len    Current string length.
 * @param[in]  code   Key code.
 * @param[in]  flags  Input flags (e.g., numeric, multiline).
 * @return            Number of bytes written.
 */
int qgl_key_parse(char *target, size_t len,
                  unsigned short code, int flags);

/** @} */

#endif /* QGL_H */
