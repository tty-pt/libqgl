/**
 * 05_ui_layout.c - UI layout system example
 * 
 * Demonstrates:
 * - Creating a UI element tree with qui_new()
 * - Using flexbox layout (row and column)
 * - Setting element dimensions and styles
 * - Computing layout with qui_layout()
 * - Rendering UI elements with qui_render()
 * 
 * To compile:
 *   cc -o 05_ui_layout 05_ui_layout.c -lqgl
 */

#include <ttypt/qgl.h>
#include <ttypt/qgl-ui.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	uint32_t w, h;
	qui_div_t *root, *header, *content, *footer;
	
	/* Get screen dimensions */
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Fill background */
	qgl_fill(0, 0, w, h, 0xFF000000);
	
	/* Create root container with flexbox column layout */
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(root, flex_direction, QUI_COLUMN);
	QUI_STYLE(root, width, 600);
	QUI_STYLE(root, height, 400);
	
	/* Create header */
	header = qui_new(root, NULL);
	QUI_STYLE(header, height, 60);
	QUI_STYLE(header, background_color, 0xFF4A4A4A);
	QUI_STYLE(header, padding_left, 20);
	QUI_STYLE(header, padding_top, 20);
	qui_text(header, "Header");
	
	/* Create content area (grows to fill space) */
	content = qui_new(root, NULL);
	QUI_STYLE(content, flex_grow, 1.0f);
	QUI_STYLE(content, background_color, 0xFFEEEEEE);
	QUI_STYLE(content, padding_left, 20);
	QUI_STYLE(content, padding_top, 20);
	qui_text(content, "Content Area");
	
	/* Create footer */
	footer = qui_new(root, NULL);
	QUI_STYLE(footer, height, 40);
	QUI_STYLE(footer, background_color, 0xFF4A4A4A);
	QUI_STYLE(footer, padding_left, 20);
	QUI_STYLE(footer, padding_top, 10);
	qui_text(footer, "Footer");
	
	/* Compute layout */
	qui_layout(root);
	
	/* Print computed positions */
	printf("Header: x=%d y=%d w=%d h=%d\n",
	       qui_get_x(header), qui_get_y(header),
	       qui_get_width(header), qui_get_height(header));
	printf("Content: x=%d y=%d w=%d h=%d\n",
	       qui_get_x(content), qui_get_y(content),
	       qui_get_width(content), qui_get_height(content));
	printf("Footer: x=%d y=%d w=%d h=%d\n",
	       qui_get_x(footer), qui_get_y(footer),
	       qui_get_width(footer), qui_get_height(footer));
	
	/* Render UI */
	qui_render(root);
	
	/* Clean up */
	qui_clear(root);
	free(root);
	
	/* Present the frame */
	qgl_flush();
	qgl_poll();
	
	return 0;
}
