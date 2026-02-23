#ifndef QGL_FONT_H
#define QGL_FONT_H

/**
 * @file qgl-font.h
 * @brief Public header for the QGL bitmap font system.
 *
 * Declares the API for loading, measuring and rendering
 * bitmap-based fonts used by the QGL rendering layer.
 */

#include <stdint.h>
#include "./qgl-ui.h"

/** @defgroup qgl_font_core QGL font management
 *  @brief Font loading and lifecycle functions.
 *  @{
 */

/**
 * @brief Load a bitmap font from an image file.
 *
 * Loads a pre-rendered font atlas (e.g. generated from BDF)
 * and registers it in QGL’s internal font table.
 *
 * @param[in] png_path Path to the font atlas (PNG).
 * @param[in] cell_w   Width of each glyph cell.
 * @param[in] cell_h   Height of each glyph cell.
 * @param[in] first    First character code in the atlas.
 * @param[in] last     Last character code in the atlas.
 * @return Font handle (font_ref) or QM_MISS on error.
 */
uint32_t qgl_font_open(const char *png_path,
		       unsigned cell_w,
		       unsigned cell_h,
		       uint8_t first,
		       uint8_t last);

/**
 * @brief Unload a font and free its resources.
 *
 * @param[in] font_ref Font handle returned by qgl_font_open().
 */
void qgl_font_close(uint32_t font_ref);

/** @} */

/** @defgroup qgl_font_render QGL font rendering and measurement
 *  @brief Functions for drawing and measuring text.
 *  @{
 */

/**
 * @brief Draw text using a loaded bitmap font.
 *
 * Renders text within the given rectangle, obeying wrapping
 * and breaking rules. Returns a pointer to the first
 * unrendered character if overflow occurred.
 *
 * @param[in] font_ref    Font handle.
 * @param[in] text        UTF-8 text to render.
 * @param[in] x0,y0       Top-left position (pixels).
 * @param[in] x1,y1       Bottom-right boundary (pixels).
 * @param[in] scale       Font scale factor.
 * @param[in] white_space White-space handling mode (see qui_white_space_t).
 * @param[in] word_break  Word-break behavior (see qui_word_break_t).
 * @return NULL if fully rendered, or pointer to remaining text.
 */
const char *qgl_font_draw(uint32_t font_ref,
			  const char *text,
			  uint32_t x0,
			  uint32_t y0,
			  uint32_t x1,
			  uint32_t y1,
			  uint32_t scale,
			  qui_white_space_t white_space,
			  qui_word_break_t word_break);

/**
 * @brief Measure text layout and overflow.
 *
 * Computes how text fits within the specified rectangle
 * and returns a pointer to the first unrendered character
 * if overflow occurred.
 *
 * @param[out] out_w       Optional: resulting text width in pixels.
 * @param[out] out_h       Optional: resulting text height in pixels.
 * @param[in]  font_ref    Font handle.
 * @param[in]  text        UTF-8 text to measure.
 * @param[in]  x0,y0       Top-left position (pixels).
 * @param[in]  x1,y1       Bottom-right boundary (pixels).
 * @param[in]  scale       Font scale factor.
 * @param[in]  white_space White-space handling mode (see qui_white_space_t).
 * @param[in]  word_break  Word-break behavior (see qui_word_break_t).
 * @return NULL if text fully fits, or pointer to remaining text.
 */
const char *qgl_font_measure(uint32_t *out_w,
			     uint32_t *out_h,
			     uint32_t font_ref,
			     const char *text,
			     uint32_t x0,
			     uint32_t y0,
			     uint32_t x1,
			     uint32_t y1,
			     uint32_t scale,
			     qui_white_space_t white_space,
			     qui_word_break_t word_break);

/** @} */

#endif /* QGL_FONT_H */
