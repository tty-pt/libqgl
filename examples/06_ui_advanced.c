/**
 * 06_ui_advanced.c - Advanced UI with styling and nested layouts
 * 
 * Demonstrates:
 * - Creating a stylesheet with qui_stylesheet_init()
 * - Defining style classes with qui_stylesheet_add()
 * - Applying classes to elements with qui_class()
 * - Nested flexbox layouts (sidebar + main area)
 * - Border radius and box shadows
 * - Responsive layouts with flex_grow
 * 
 * To compile:
 *   cc -o 06_ui_advanced 06_ui_advanced.c -lqgl
 */

#include <ttypt/qgl.h>
#include <ttypt/qgl-ui.h>
#include <ttypt/qgl-font.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	uint32_t w, h;
	uint32_t ss;
	qui_div_t *root, *sidebar, *main_area, *header, *content;
	qui_style_t sidebar_style, header_style, content_style, button_style;
	
	/* Get screen dimensions */
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Fill background */
	qgl_fill(0, 0, w, h, 0xFF0A0A0A);
	
	/* Create stylesheet */
	ss = qui_stylesheet_init();
	
	/* Define sidebar style */
	qui_style_reset(&sidebar_style);
	sidebar_style.background_color = 0xFF2C3E50;
	sidebar_style.padding_top = 20;
	sidebar_style.padding_left = 15;
	sidebar_style.flex_basis = 200;
	qui_stylesheet_add(ss, "sidebar", &sidebar_style);
	
	/* Define header style */
	qui_style_reset(&header_style);
	header_style.background_color = 0xFF34495E;
	header_style.height = 80;
	header_style.padding_top = 25;
	header_style.padding_left = 30;
	header_style.box_shadow_color = 0x40000000;
	header_style.box_shadow_blur = 8.0f;
	header_style.box_shadow_offset_y = 4.0f;
	qui_stylesheet_add(ss, "header", &header_style);
	
	/* Define content style */
	qui_style_reset(&content_style);
	content_style.background_color = 0xFFECF0F1;
	content_style.flex_grow = 1.0f;
	content_style.padding_top = 30;
	content_style.padding_left = 30;
	qui_stylesheet_add(ss, "content", &content_style);
	
	/* Define button style */
	qui_style_reset(&button_style);
	button_style.background_color = 0xFF3498DB;
	button_style.width = 120;
	button_style.height = 40;
	button_style.border_radius_top_left = 5;
	button_style.border_radius_top_right = 5;
	button_style.border_radius_bottom_right = 5;
	button_style.border_radius_bottom_left = 5;
	button_style.padding_left = 15;
	button_style.padding_top = 10;
	button_style.box_shadow_color = 0x60000000;
	button_style.box_shadow_blur = 5.0f;
	button_style.box_shadow_offset_y = 2.0f;
	qui_stylesheet_add(ss, "button", &button_style);
	
	/* Create root container (horizontal split) */
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(root, flex_direction, QUI_ROW);
	QUI_STYLE(root, width, 800);
	QUI_STYLE(root, height, 600);
	
	/* Create sidebar */
	sidebar = qui_new(root, NULL);
	qui_class(sidebar, "sidebar");
	qui_text(sidebar, "Navigation");
	
	/* Create main area (vertical split) */
	main_area = qui_new(root, NULL);
	QUI_STYLE(main_area, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(main_area, flex_direction, QUI_COLUMN);
	QUI_STYLE(main_area, flex_grow, 1.0f);
	
	/* Create header */
	header = qui_new(main_area, NULL);
	qui_class(header, "header");
	qui_text(header, "Dashboard");
	
	/* Create content area */
	content = qui_new(main_area, NULL);
	qui_class(content, "content");
	qui_text(content, "Main Content");
	
	/* Add some button-like elements to content */
	qui_div_t *button1 = qui_new(content, NULL);
	qui_class(button1, "button");
	qui_text(button1, "Click Me");
	
	qui_div_t *button2 = qui_new(content, NULL);
	qui_class(button2, "button");
	qui_text(button2, "Action");
	QUI_STYLE(button2, background_color, 0xFF2ECC71);  /* Green button */
	
	/* Apply styles from stylesheet */
	qui_apply_styles(root, ss);
	
	/* Compute layout */
	qui_layout(root);
	
	/* Print layout info */
	printf("Sidebar: %dx%d at (%d,%d)\n",
	       qui_get_width(sidebar), qui_get_height(sidebar),
	       qui_get_x(sidebar), qui_get_y(sidebar));
	printf("Header: %dx%d at (%d,%d)\n",
	       qui_get_width(header), qui_get_height(header),
	       qui_get_x(header), qui_get_y(header));
	printf("Content: %dx%d at (%d,%d)\n",
	       qui_get_width(content), qui_get_height(content),
	       qui_get_x(content), qui_get_y(content));
	
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
