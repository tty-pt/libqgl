/**
 * 05_ui_layout.c - UI layout system example
 * 
 * Demonstrates:
 * - Creating a UI element tree with lam_ui_new()
 * - Using flexbox layout (row and column)
 * - Setting element dimensions and styles
 * - Computing layout with lam_ui_layout()
 * - Rendering UI elements with lam_ui_render()
 * 
 * To compile:
 *   cc -o 05_ui_layout 05_ui_layout.c -llamina
 */

#include <ttypt/lam.h>
#include <ttypt/lam-ui.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	uint32_t w, h;
	lam_ui_div_t *root, *header, *content, *footer;
	
	/* Get screen dimensions */
	lam_size(&w, &h);
	lam_ui_init(w, h);
	
	/* Fill background */
	lam_fill(0, 0, w, h, 0xFF000000);
	
	/* Create root container with flexbox column layout */
	root = lam_ui_new(NULL, NULL);
	LAM_UI_STYLE(root, display, LAM_UI_DISPLAY_FLEX);
	LAM_UI_STYLE(root, flex_direction, LAM_UI_COLUMN);
	LAM_UI_STYLE(root, width, 600);
	LAM_UI_STYLE(root, height, 400);
	
	/* Create header */
	header = lam_ui_new(root, NULL);
	LAM_UI_STYLE(header, height, 60);
	LAM_UI_STYLE(header, background_color, 0xFF4A4A4A);
	LAM_UI_STYLE(header, padding_left, 20);
	LAM_UI_STYLE(header, padding_top, 20);
	lam_ui_text(header, "Header");
	
	/* Create content area (grows to fill space) */
	content = lam_ui_new(root, NULL);
	LAM_UI_STYLE(content, flex_grow, 1.0f);
	LAM_UI_STYLE(content, background_color, 0xFFEEEEEE);
	LAM_UI_STYLE(content, padding_left, 20);
	LAM_UI_STYLE(content, padding_top, 20);
	lam_ui_text(content, "Content Area");
	
	/* Create footer */
	footer = lam_ui_new(root, NULL);
	LAM_UI_STYLE(footer, height, 40);
	LAM_UI_STYLE(footer, background_color, 0xFF4A4A4A);
	LAM_UI_STYLE(footer, padding_left, 20);
	LAM_UI_STYLE(footer, padding_top, 10);
	lam_ui_text(footer, "Footer");
	
	/* Compute layout */
	lam_ui_layout(root);
	
	/* Print computed positions */
	printf("Header: x=%d y=%d w=%d h=%d\n",
	       lam_ui_get_x(header), lam_ui_get_y(header),
	       lam_ui_get_width(header), lam_ui_get_height(header));
	printf("Content: x=%d y=%d w=%d h=%d\n",
	       lam_ui_get_x(content), lam_ui_get_y(content),
	       lam_ui_get_width(content), lam_ui_get_height(content));
	printf("Footer: x=%d y=%d w=%d h=%d\n",
	       lam_ui_get_x(footer), lam_ui_get_y(footer),
	       lam_ui_get_width(footer), lam_ui_get_height(footer));
	
	/* Render UI */
	lam_ui_render(root);
	
	/* Clean up */
	lam_ui_clear(root);
	free(root);
	
	/* Present the frame */
	lam_flush();
	lam_poll();
	
	return 0;
}
