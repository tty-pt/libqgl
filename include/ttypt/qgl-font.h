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
 * qgl_font_open - Load a bitmap font from an image file
 * @png_path: Path to the font atlas (PNG)
 * @cell_w:   Width of each glyph cell
 * @cell_h:   Height of each glyph cell
 * @first:    First character code in the atlas
 * @last:     Last character code in the atlas
 *
 * Loads a pre-rendered font atlas (e.g. generated from BDF)
 * and registers it in QGL’s internal font table.
 *
 * Return: Font handle (font_ref) or QM_MISS on error.
 */
uint32_t qgl_font_open(const char *png_path,
		       unsigned cell_w,
		       unsigned cell_h,
		       uint8_t first,
		       uint8_t last);

/**
 * qgl_font_close - Unload a font and free its resources
 * @font_ref: Font handle returned by qgl_font_open()
 */
void qgl_font_close(uint32_t font_ref);

/** @} */

/** @defgroup qgl_font_render QGL font rendering and measurement
 *  @brief Functions for drawing and measuring text.
 *  @{
 */

/**
 * qgl_font_draw - Draw text using a loaded bitmap font
 * @font_ref:     Font handle
 * @text:         UTF-8 text to render
 * @x0, y0:       Top-left position (pixels)
 * @x1, y1:       Bottom-right boundary (pixels)
 * @scale:        Font scale factor
 * @white_space:  White-space handling mode (see qui_white_space_t)
 * @word_break:   Word-break behavior (see qui_word_break_t)
 *
 * Renders text within the given rectangle, obeying wrapping
 * and breaking rules. Returns a pointer to the first
 * unrendered character if overflow occurred.
 *
 * Return: NULL if fully rendered, or pointer to remaining text.
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
 * qgl_font_measure - Measure text layout and overflow
 * @out_w:        Optional: resulting text width in pixels
 * @out_h:        Optional: resulting text height in pixels
 * @font_ref:     Font handle
 * @text:         UTF-8 text to measure
 * @x0, y0:       Top-left position (pixels)
 * @x1, y1:       Bottom-right boundary (pixels)
 * @scale:        Font scale factor
 * @white_space:  White-space handling mode (see qui_white_space_t)
 * @word_break:   Word-break behavior (see qui_word_break_t)
 *
 * Computes how text fits within the specified rectangle
 * and returns a pointer to the first unrendered character
 * if overflow occurred.
 *
 * Return: NULL if text fully fits, or pointer to remaining text.
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
