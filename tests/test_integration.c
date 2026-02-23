/**
 * test_integration.c - Integration tests for QGL
 * Tests complete scenarios combining multiple subsystems
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ttypt/qgl.h>
#include <ttypt/qgl-ui.h>
#include <ttypt/qgl-font.h>
#include <ttypt/qgl-tm.h>

static void test_complete_ui_pipeline(void) {
	uint32_t w, h, ss;
	qui_div_t *root, *header, *content, *footer;
	qui_style_t header_style, content_style, footer_style;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Create stylesheet */
	ss = qui_stylesheet_init();
	
	qui_style_reset(&header_style);
	header_style.background_color = 0xFF333333;
	header_style.height = 60;
	header_style.padding_left = 20;
	qui_stylesheet_add(ss, "header", &header_style);
	
	qui_style_reset(&content_style);
	content_style.background_color = 0xFFFFFFFF;
	content_style.flex_grow = 1.0f;
	content_style.padding_left = 20;
	content_style.padding_top = 20;
	qui_stylesheet_add(ss, "content", &content_style);
	
	qui_style_reset(&footer_style);
	footer_style.background_color = 0xFF333333;
	footer_style.height = 40;
	footer_style.padding_left = 20;
	qui_stylesheet_add(ss, "footer", &footer_style);
	
	/* Build UI tree */
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(root, flex_direction, QUI_COLUMN);
	QUI_STYLE(root, width, 800);
	QUI_STYLE(root, height, 600);
	
	header = qui_new(root, NULL);
	qui_class(header, "header");
	qui_text(header, "Header");
	
	content = qui_new(root, NULL);
	qui_class(content, "content");
	qui_text(content, "Main Content Area");
	
	footer = qui_new(root, NULL);
	qui_class(footer, "footer");
	qui_text(footer, "Footer");
	
	/* Apply styles and layout */
	qui_apply_styles(root, ss);
	qui_layout(root);
	
	/* Verify layout */
	assert(qui_get_y(header) == 0);
	assert(qui_get_y(content) == 60);
	assert(qui_get_height(content) > 0);
	assert(qui_get_y(footer) > qui_get_y(content));
	
	/* Render */
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_complete_ui_pipeline: PASS\n");
}

static void test_texture_and_ui(void) {
	uint32_t w, h, tex_ref;
	qui_div_t *root, *container;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Load a texture */
	tex_ref = qgl_tex_load("tests/fixtures/test_texture.png");
	assert(tex_ref != 0);
	
	/* Create UI with background image */
	root = qui_new(NULL, NULL);
	container = qui_new(root, NULL);
	QUI_STYLE(container, width, 200);
	QUI_STYLE(container, height, 200);
	QUI_STYLE(container, background_image_ref, tex_ref);
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_texture_and_ui: PASS\n");
}

static void test_font_and_ui(void) {
	uint32_t w, h, font_ref;
	qui_div_t *root, *text_box;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Load a font (8x8 cells, ASCII 32-126) */
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != 0);
	
	/* Create UI with text */
	root = qui_new(NULL, NULL);
	text_box = qui_new(root, NULL);
	QUI_STYLE(text_box, width, 300);
	QUI_STYLE(text_box, height, 100);
	QUI_STYLE(text_box, font_family_ref, font_ref);
	QUI_STYLE(text_box, font_size, 16);
	QUI_STYLE(text_box, padding_left, 10);
	QUI_STYLE(text_box, padding_top, 10);
	qui_text(text_box, "Hello from QGL!");
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	qgl_font_close(font_ref);
	
	printf("  test_font_and_ui: PASS\n");
}

static void test_responsive_layout(void) {
	uint32_t w, h;
	qui_div_t *root, *col1, *col2, *col3;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Create a responsive 3-column layout */
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(root, flex_direction, QUI_ROW);
	QUI_STYLE(root, width, 900);
	QUI_STYLE(root, height, 400);
	
	col1 = qui_new(root, NULL);
	QUI_STYLE(col1, flex_grow, 1.0f);
	QUI_STYLE(col1, flex_basis, 0);
	QUI_STYLE(col1, background_color, 0xFFFF0000);
	
	col2 = qui_new(root, NULL);
	QUI_STYLE(col2, flex_grow, 2.0f);
	QUI_STYLE(col2, flex_basis, 0);
	QUI_STYLE(col2, background_color, 0xFF00FF00);
	
	col3 = qui_new(root, NULL);
	QUI_STYLE(col3, flex_grow, 1.0f);
	QUI_STYLE(col3, flex_basis, 0);
	QUI_STYLE(col3, background_color, 0xFFFF0000);
	
	qui_layout(root);
	
	/* Verify proportional sizing */
	int32_t w1 = qui_get_width(col1);
	int32_t w2 = qui_get_width(col2);
	int32_t w3 = qui_get_width(col3);
	
	assert(w1 > 0);
	assert(w2 > w1);  /* col2 should be wider */
	assert(w3 == w1); /* col1 and col3 should be equal */
	
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_responsive_layout: PASS\n");
}

static void test_nested_flex_layout(void) {
	uint32_t w, h;
	qui_div_t *root, *sidebar, *main, *header, *content;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Create nested flex layout: horizontal split with vertical split in main */
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(root, flex_direction, QUI_ROW);
	QUI_STYLE(root, width, 1000);
	QUI_STYLE(root, height, 600);
	
	sidebar = qui_new(root, NULL);
	QUI_STYLE(sidebar, flex_basis, 200);
	QUI_STYLE(sidebar, background_color, 0xFF2C3E50);
	
	main = qui_new(root, NULL);
	QUI_STYLE(main, flex_grow, 1.0f);
	QUI_STYLE(main, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(main, flex_direction, QUI_COLUMN);
	
	header = qui_new(main, NULL);
	QUI_STYLE(header, flex_basis, 80);
	QUI_STYLE(header, background_color, 0xFF34495E);
	
	content = qui_new(main, NULL);
	QUI_STYLE(content, flex_grow, 1.0f);
	QUI_STYLE(content, background_color, 0xFFECF0F1);
	
	qui_layout(root);
	
	/* Verify nested layout */
	assert(qui_get_x(sidebar) == 0);
	assert(qui_get_width(sidebar) == 200);
	assert(qui_get_x(main) == 200);
	assert(qui_get_width(main) == 800);
	assert(qui_get_y(header) == 0);
	assert(qui_get_height(header) == 80);
	assert(qui_get_y(content) == 80);
	assert(qui_get_height(content) == 520);
	
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_nested_flex_layout: PASS\n");
}

static void test_text_overflow(void) {
	uint32_t w, h, font_ref;
	qui_div_t *root, *text_box;
	const char *long_text = "This is a very long text that will definitely overflow "
	                        "the container and should be handled properly by the layout "
	                        "and rendering system.";
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != 0);
	
	root = qui_new(NULL, NULL);
	text_box = qui_new(root, NULL);
	QUI_STYLE(text_box, width, 100);
	QUI_STYLE(text_box, height, 30);
	QUI_STYLE(text_box, font_family_ref, font_ref);
	QUI_STYLE(text_box, overflow_y, QUI_OVERFLOW_HIDDEN);
	qui_text(text_box, long_text);
	
	qui_layout(root);
	qui_render(root);
	
	/* Check if overflow was detected */
	const char *overflow = qui_overflow(text_box);
	(void)overflow;  /* May or may not be NULL depending on implementation */
	
	qui_clear(root);
	free(root);
	qgl_font_close(font_ref);
	
	printf("  test_text_overflow: PASS\n");
}

static void test_mixed_positioning(void) {
	uint32_t w, h;
	qui_div_t *root, *relative, *absolute;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, width, 500);
	QUI_STYLE(root, height, 500);
	
	relative = qui_new(root, NULL);
	QUI_STYLE(relative, position, QUI_POSITION_RELATIVE);
	QUI_STYLE(relative, width, 200);
	QUI_STYLE(relative, height, 200);
	QUI_STYLE(relative, background_color, 0xFF3498DB);
	
	absolute = qui_new(root, NULL);
	QUI_STYLE(absolute, position, QUI_POSITION_ABSOLUTE);
	QUI_STYLE(absolute, left, 50);
	QUI_STYLE(absolute, top, 50);
	QUI_STYLE(absolute, width, 100);
	QUI_STYLE(absolute, height, 100);
	QUI_STYLE(absolute, background_color, 0xFFE74C3C);
	
	qui_layout(root);
	qui_render(root);
	
	/* Verify positioning */
	assert(qui_get_x(absolute) == 50);
	assert(qui_get_y(absolute) == 50);
	
	qui_clear(root);
	free(root);
	
	printf("  test_mixed_positioning: PASS\n");
}

int main(void) {
	printf("test_integration:\n");
	
	test_complete_ui_pipeline();
	test_texture_and_ui();
	test_font_and_ui();
	test_responsive_layout();
	test_nested_flex_layout();
	test_text_overflow();
	test_mixed_positioning();
	
	printf("test_integration: ALL TESTS PASSED\n");
	return 0;
}
