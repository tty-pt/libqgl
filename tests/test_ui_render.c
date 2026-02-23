/**
 * test_ui_render.c - UI Rendering tests for QGL
 * Tests rendering, caching, and visual output
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ttypt/qgl.h>
#include <ttypt/qgl-ui.h>
#include <ttypt/qgl-font.h>

static void test_qui_render_basic(void) {
	uint32_t w, h;
	qui_div_t *root;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, width, 100);
	QUI_STYLE(root, height, 100);
	QUI_STYLE(root, background_color, 0xFF0000FF);
	
	qui_layout(root);
	
	/* Render should not crash */
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_basic: PASS\n");
}

static void test_qui_render_text(void) {
	uint32_t w, h;
	qui_div_t *root;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, width, 200);
	QUI_STYLE(root, height, 50);
	qui_text(root, "Hello World");
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_text: PASS\n");
}

static void test_qui_render_nested(void) {
	uint32_t w, h;
	qui_div_t *root, *child1, *child2;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(root, width, 400);
	QUI_STYLE(root, height, 200);
	
	child1 = qui_new(root, NULL);
	QUI_STYLE(child1, flex_basis, 100);
	QUI_STYLE(child1, background_color, 0xFF0000FF);
	
	child2 = qui_new(root, NULL);
	QUI_STYLE(child2, flex_basis, 100);
	QUI_STYLE(child2, background_color, 0xFF00FF00);
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_nested: PASS\n");
}

static void test_qui_mark_dirty(void) {
	uint32_t w, h;
	qui_div_t *root, *child;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	child = qui_new(root, NULL);
	
	qui_layout(root);
	qui_render(root);
	
	/* Mark dirty and re-render */
	qui_mark_dirty(child);
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_mark_dirty: PASS\n");
}

static void test_qui_render_border(void) {
	uint32_t w, h;
	qui_div_t *root;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, width, 100);
	QUI_STYLE(root, height, 100);
	QUI_STYLE(root, background_color, 0xFFFFFFFF);
	QUI_STYLE(root, border_width, 5);
	QUI_STYLE(root, border_color, 0xFF000000);
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_border: PASS\n");
}

static void test_qui_render_border_radius(void) {
	uint32_t w, h;
	qui_div_t *root;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, width, 100);
	QUI_STYLE(root, height, 100);
	QUI_STYLE(root, background_color, 0xFFFF0000);
	QUI_STYLE(root, border_width, 2);
	QUI_STYLE(root, border_color, 0xFF000000);
	QUI_STYLE(root, border_radius_top_left, 10);
	QUI_STYLE(root, border_radius_top_right, 10);
	QUI_STYLE(root, border_radius_bottom_right, 10);
	QUI_STYLE(root, border_radius_bottom_left, 10);
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_border_radius: PASS\n");
}

static void test_qui_render_padding(void) {
	uint32_t w, h;
	qui_div_t *root, *child;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, width, 200);
	QUI_STYLE(root, height, 200);
	QUI_STYLE(root, background_color, 0xFFCCCCCC);
	QUI_STYLE(root, padding_top, 20);
	QUI_STYLE(root, padding_right, 20);
	QUI_STYLE(root, padding_bottom, 20);
	QUI_STYLE(root, padding_left, 20);
	
	child = qui_new(root, NULL);
	QUI_STYLE(child, width, 100);
	QUI_STYLE(child, height, 100);
	QUI_STYLE(child, background_color, 0xFF0000FF);
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_padding: PASS\n");
}

static void test_qui_render_box_shadow(void) {
	uint32_t w, h;
	qui_div_t *root;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, width, 100);
	QUI_STYLE(root, height, 100);
	QUI_STYLE(root, background_color, 0xFFFFFFFF);
	QUI_STYLE(root, box_shadow_color, 0x80000000);  /* Semi-transparent black */
	QUI_STYLE(root, box_shadow_blur, 10.0f);
	QUI_STYLE(root, box_shadow_offset_x, 5.0f);
	QUI_STYLE(root, box_shadow_offset_y, 5.0f);
	
	qui_layout(root);
	qui_render(root);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_box_shadow: PASS\n");
}

static void test_qui_render_display_none(void) {
	uint32_t w, h;
	qui_div_t *root, *hidden_child, *visible_child;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	QUI_STYLE(root, display, QUI_DISPLAY_FLEX);
	
	hidden_child = qui_new(root, NULL);
	QUI_STYLE(hidden_child, display, QUI_DISPLAY_NONE);
	QUI_STYLE(hidden_child, width, 100);
	QUI_STYLE(hidden_child, height, 100);
	
	visible_child = qui_new(root, NULL);
	QUI_STYLE(visible_child, width, 100);
	QUI_STYLE(visible_child, height, 100);
	QUI_STYLE(visible_child, background_color, 0xFF00FF00);
	
	qui_layout(root);
	qui_render(root);
	
	/* Hidden child should not affect layout */
	assert(qui_get_x(visible_child) == 0);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_render_display_none: PASS\n");
}

static void test_qui_clear(void) {
	uint32_t w, h;
	qui_div_t *root, *child1, *child2;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	child1 = qui_new(root, NULL);
	child2 = qui_new(root, NULL);
	
	/* Add grandchildren */
	qui_new(child1, NULL);
	qui_new(child1, NULL);
	qui_new(child2, NULL);
	
	/* Clear should recursively free all children */
	qui_clear(root);
	free(root);
	
	printf("  test_qui_clear: PASS\n");
}

int main(void) {
	printf("test_ui_render:\n");
	
	test_qui_render_basic();
	test_qui_render_text();
	test_qui_render_nested();
	test_qui_mark_dirty();
	test_qui_render_border();
	test_qui_render_border_radius();
	test_qui_render_padding();
	test_qui_render_box_shadow();
	test_qui_render_display_none();
	test_qui_clear();
	
	printf("test_ui_render: ALL TESTS PASSED\n");
	return 0;
}
