#ifndef QGL_H
#define QGL_H

/**
 * @file qgl.h
 * @brief Public header for the QGL rendering layer.
 *
 * QGL provides a small, portable 2D rendering API that
 * abstracts over multiple backends (OpenGL, framebuffer,
 * software) without exposing platform-specific details.
 *
 * It supports:
 *  - CPU pixel rendering (lambda functions)
 *  - texture/sprite batching
 *  - tint and color manipulation
 *  - input polling and key callbacks
 */

#include <stdint.h>

#include "./qgl-key.h"

/*───────────────────────────────────────────────*
 *              CONSTANTS & TYPES                *
 *───────────────────────────────────────────────*/

/** @defgroup qgl_types QGL types and constants
 *  @brief Type definitions and constants used by QGL.
 *  @{
 */

/**
 * @brief CPU-side render callback.
 *
 * Invoked per pixel inside a rectangular region.
 * Implementations can render procedurally into the
 * current canvas.
 *
 * @param[out] color Pointer to BGRA pixel data.
 * @param[in]  x     Pixel X coordinate.
 * @param[in]  y     Pixel Y coordinate.
 * @param[in]  ctx   Optional user context pointer.
 */
typedef void qgl_lambda_t(uint8_t *color,
                          uint32_t x, uint32_t y,
                          void *ctx);

/**
 * @brief Query the current framebuffer size.
 *
 * Provides the logical size of the active backend
 * (e.g., window framebuffer, offscreen buffer,
 * or display surface).  Implementations may adjust
 * the values dynamically on resize.
 *
 * @param[out] width  Framebuffer width in pixels.
 * @param[out] height Framebuffer height in pixels.
 */
void qgl_size(uint32_t *width, uint32_t *height);

/** Default white tint (0xFFFFFFFF RGBA). */
static const uint32_t qgl_default_tint = 0xFFFFFFFF;

/**
 * @brief Keyboard event callback signature.
 *
 * @param[in] code  Platform key code.
 * @param[in] type  Event type (press, release, repeat).
 * @param[in] value Event value (1 = pressed, 0 = released).
 * @return          Non-zero to consume the event.
 */
typedef int qgl_key_cb_t(unsigned short code,
                         unsigned short type,
                         int value);

/** @} */

/*───────────────────────────────────────────────*
 *                 RENDERING API                 *
 *───────────────────────────────────────────────*/

/** @defgroup qgl_render QGL rendering
 *  @brief Functions for rendering and presentation.
 *  @{
 */

/**
 * @brief Render pixels using a user-provided callback.
 *
 * Iterates over a rectangular region, invoking a callback
 * per pixel. Useful for software or procedural rendering
 * independent of backend.
 *
 * @param[in] lambda Callback executed per pixel.
 * @param[in] x,y    Region offset in pixels.
 * @param[in] w,h    Region dimensions.
 * @param[in] ctx    Optional user context.
 */
void qgl_render(qgl_lambda_t *lambda,
                int32_t x, int32_t y,
                uint32_t w, uint32_t h,
                void *ctx);

/**
 * @brief Flush the current canvas to the active backend.
 *
 * Uploads the canvas (if used), executes any pending
 * draw calls, and presents the framebuffer through the
 * selected backend (OpenGL, FB, or software).
 */
void qgl_flush(void);

/** @} */

/*───────────────────────────────────────────────*
 *              TEXTURE / SPRITE API             *
 *───────────────────────────────────────────────*/

/** @defgroup qgl_texture QGL textures and sprites
 *  @brief Portable texture and sprite drawing interface.
 *  @{
 */

/**
 * @brief Draw a sub-region of a texture.
 *
 * @param[in] ref Texture reference handle.
 * @param[in] x,y Destination position.
 * @param[in] cx,cy Source region origin.
 * @param[in] sw,sh Source region size.
 * @param[in] dw,dh Destination size in pixels.
 * @param[in] tint RGBA color multiplier.
 */
void qgl_tex_draw_x(uint32_t ref,
                    int32_t x, int32_t y,
                    uint32_t cx, uint32_t cy,
                    uint32_t sw, uint32_t sh,
                    uint32_t dw, uint32_t dh,
                    uint32_t tint);

/**
 * @brief Draw a full texture scaled to a destination area.
 *
 * @param[in] ref Texture reference handle.
 * @param[in] x,y Destination position.
 * @param[in] dw,dh Destination width and height.
 */
void qgl_tex_draw(uint32_t ref,
                  int32_t x, int32_t y,
                  uint32_t dw, uint32_t dh);

/**
 * @brief Load an image file as a texture.
 *
 * The supported image formats depend on the backend
 * (e.g., PNG for OpenGL, raw BGRA for framebuffer).
 *
 * @param[in] filename Path to the image file.
 * @return             Texture reference ID.
 */
unsigned qgl_tex_load(const char *filename);

/**
 * @brief Save a texture to disk (optional backend feature).
 *
 * @param[in] ref Texture reference handle.
 */
void qgl_tex_save(unsigned ref);

/**
 * @brief Retrieve a texture’s size in pixels.
 *
 * @param[out] w Width in pixels.
 * @param[out] h Height in pixels.
 * @param[in]  ref Texture reference handle.
 */
void qgl_tex_size(uint32_t *w, uint32_t *h, unsigned ref);

/**
 * @brief Read a single pixel from a texture.
 *
 * @param[in] ref Texture reference handle.
 * @param[in] x,y Pixel coordinates within the texture.
 * @return        32-bit BGRA color.
 */
uint32_t qgl_tex_pick(unsigned ref, uint32_t x, uint32_t y);

/**
 * @brief Modify a single pixel within a texture.
 *
 * @param[in] ref   Texture reference handle.
 * @param[in] x,y   Target pixel coordinates.
 * @param[in] color 32-bit BGRA color.
 */
void qgl_tex_paint(unsigned ref,
                   uint32_t x, uint32_t y,
                   uint32_t color);

/**
 * @brief Apply a global RGBA tint to future draw calls.
 *
 * @param[in] tint 32-bit RGBA multiplier.
 */
void qgl_tint(uint32_t tint);

/** @} */

/*───────────────────────────────────────────────*
 *                 INPUT API                     *
 *───────────────────────────────────────────────*/

/** @defgroup qgl_input QGL input handling
 *  @brief Keyboard event polling and registration.
 *  @{
 */

/**
 * @brief Poll input events from the active backend.
 *
 * Reads pending input from the current platform
 * and dispatches them to registered callbacks.
 */
void qgl_poll(void);

/**
 * @brief Register a callback for a specific key code.
 *
 * @param[in] key Key code (backend-specific).
 * @param[in] cb  Function pointer to handle key events.
 */
void qgl_key_reg(unsigned short key, qgl_key_cb_t *cb);

/**
 * @brief Register a callback to be called for all keys.
 *
 * @param[in] cb  Function pointer to handle key events.
 */
void qgl_key_default_reg(qgl_key_cb_t *cb);

/**
 * @brief Register a callback to be called for all keys.
 *
 * @param[in] cb  Function pointer to handle key events.
 */
unsigned short qgl_key_val(unsigned short code);

/** @} */

#endif /* QGL_H */
