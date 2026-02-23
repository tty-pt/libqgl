/**
 * 03_fonts.c - Bitmap font rendering example
 * 
 * Demonstrates:
 * - Loading bitmap fonts from PNG atlases
 * - Rendering text at different positions
 * - Measuring text dimensions
 * - Text wrapping and overflow handling
 * - Using different font sizes
 * 
 * To compile:
 *   cc -o 03_fonts 03_fonts.c -lqgl
 * 
 * Note: Requires a font atlas PNG (e.g., ../tests/fixtures/test_font.png)
 */

#include <ttypt/qgl.h>
#include <ttypt/qgl-font.h>
#include <ttypt/qgl-ui.h>
#include <stdio.h>

int main(void)
{
	uint32_t w, h;
	uint32_t font_ref;
	uint32_t text_w, text_h;
	const char *text = "Hello, QGL!";
	const char *long_text = "This is a longer text that demonstrates text "
	                        "wrapping and overflow handling in the QGL font system.";
	
	/* Get screen dimensions */
	qgl_size(&w, &h);
	
	/* Fill background */
	qgl_fill(0, 0, w, h, 0xFF1E1E1E);
	
	/* Load a bitmap font (8x8 cells, ASCII 32-126) */
	font_ref = qgl_font_open("../tests/fixtures/test_font.png", 8, 8, 32, 126);
	if (font_ref == 0) {
		printf("Failed to load font\n");
		return 1;
	}
	
	/* Measure text - bounding box (0,0) to (w,h) */
	qgl_font_measure(&text_w, &text_h, font_ref, text, 
	                 0, 0, w, h, 8, QUI_WS_NORMAL, QUI_WB_NORMAL);
	printf("Text '%s' dimensions: %u x %u\n", text, text_w, text_h);
	
	/* Draw text at normal size (8x8) - bounding box from (50,50) to (w,h) */
	qgl_font_draw(font_ref, text, 50, 50, w, h, 8, QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Draw text at 2x size (16x16) */
	qgl_font_draw(font_ref, "Large Text", 50, 100, w, h, 16, QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Draw text at smaller size (6x6) */
	qgl_font_draw(font_ref, "Small Text", 50, 150, w, h, 6, QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Draw text with color tint */
	qgl_tint(0xFF00FFFF);  /* Yellow */
	qgl_font_draw(font_ref, "Colored Text", 50, 200, w, h, 8, QUI_WS_NORMAL, QUI_WB_NORMAL);
	qgl_tint(qgl_default_tint);
	
	/* Demonstrate text wrapping - constrain width to 300px */
	qgl_font_draw(font_ref, long_text, 50, 250, 350, h, 8, QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Measure wrapped text - constrain width to 300px */
	qgl_font_measure(&text_w, &text_h, font_ref, long_text,
	                 50, 250, 350, h, 8, QUI_WS_NORMAL, QUI_WB_NORMAL);
	printf("Wrapped text dimensions: %u x %u\n", text_w, text_h);
	
	/* Clean up */
	qgl_font_close(font_ref);
	
	/* Present the frame */
	qgl_flush();
	qgl_poll();
	
	return 0;
}
