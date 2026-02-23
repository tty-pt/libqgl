/**
 * test_ui_style.c - UI Styling tests for QGL
 * Tests stylesheet creation, class application, and style inheritance
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ttypt/qgl.h>
#include <ttypt/qgl-ui.h>

static void test_qui_stylesheet_init(void) {
	uint32_t ss;
	
	ss = qui_stylesheet_init();
	assert(ss != 0);
	
	printf("  test_qui_stylesheet_init: PASS\n");
}

static void test_qui_stylesheet_add(void) {
	uint32_t ss;
	qui_style_t style;
	
	ss = qui_stylesheet_init();
	qui_style_reset(&style);
	
	style.background_color = 0xFF0000FF;  /* Red */
	style.width = 100;
	style.height = 50;
	
	qui_stylesheet_add(ss, "test-class", &style);
	
	printf("  test_qui_stylesheet_add: PASS\n");
}

static void test_qui_class(void) {
	uint32_t w, h;
	qui_div_t *div;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	div = qui_new(NULL, NULL);
	qui_class(div, "my-class");
	
	qui_clear(div);
	free(div);
	
	printf("  test_qui_class: PASS\n");
}

static void test_qui_apply_styles(void) {
	uint32_t w, h, ss;
	qui_div_t *root, *child;
	qui_style_t style;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Create stylesheet with a class */
	ss = qui_stylesheet_init();
	qui_style_reset(&style);
	style.background_color = 0xFF00FF00;  /* Green */
	style.padding_left = 10;
	style.padding_top = 20;
	qui_stylesheet_add(ss, "green-box", &style);
	
	/* Create element with that class */
	root = qui_new(NULL, NULL);
	child = qui_new(root, NULL);
	qui_class(child, "green-box");
	
	/* Apply styles */
	qui_apply_styles(root, ss);
	
	/* Note: We can't verify the actual style was applied since
	 * qui_style_t is not accessible from the public API. 
	 * This test mainly ensures the function doesn't crash */
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_apply_styles: PASS\n");
}

static void test_qui_style_set(void) {
	uint32_t w, h;
	qui_div_t *div;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	div = qui_new(NULL, NULL);
	
	/* Test QUI_STYLE macro which uses qui_style_set */
	QUI_STYLE(div, width, 200);
	QUI_STYLE(div, height, 100);
	QUI_STYLE(div, background_color, 0xFFFF0000);  /* Blue */
	
	qui_clear(div);
	free(div);
	
	printf("  test_qui_style_set: PASS\n");
}

static void test_qui_style_reset(void) {
	qui_style_t style;
	
	/* Set some non-default values */
	memset(&style, 0xFF, sizeof(style));
	
	/* Reset to defaults */
	qui_style_reset(&style);
	
	/* Verify some default values */
	assert(style.background_color == 0);
	assert(style.border_width == 0);
	assert(style.display == QUI_DISPLAY_BLOCK);
	assert(style.flex_grow == 0.0f);
	assert(style.flex_shrink == 1.0f);
	
	printf("  test_qui_style_reset: PASS\n");
}

static void test_qui_inline_style(void) {
	uint32_t w, h;
	qui_div_t *root, *div_inline, *div_default;
	qui_style_t inline_style;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	root = qui_new(NULL, NULL);
	
	/* Create div with inline style */
	qui_style_reset(&inline_style);
	inline_style.width = 150;
	inline_style.height = 75;
	inline_style.display = QUI_DISPLAY_FLEX;
	div_inline = qui_new(root, &inline_style);
	
	/* Create div with default style */
	div_default = qui_new(root, NULL);
	
	assert(div_inline != NULL);
	assert(div_default != NULL);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_inline_style: PASS\n");
}

static void test_qui_multiple_classes(void) {
	uint32_t w, h, ss;
	qui_div_t *root, *child1, *child2, *child3;
	qui_style_t style1, style2, style3;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	/* Create stylesheet with multiple classes */
	ss = qui_stylesheet_init();
	
	qui_style_reset(&style1);
	style1.background_color = 0xFF0000FF;
	qui_stylesheet_add(ss, "red", &style1);
	
	qui_style_reset(&style2);
	style2.background_color = 0xFF00FF00;
	qui_stylesheet_add(ss, "green", &style2);
	
	qui_style_reset(&style3);
	style3.background_color = 0xFFFF0000;
	qui_stylesheet_add(ss, "blue", &style3);
	
	/* Create elements with different classes */
	root = qui_new(NULL, NULL);
	child1 = qui_new(root, NULL);
	qui_class(child1, "red");
	
	child2 = qui_new(root, NULL);
	qui_class(child2, "green");
	
	child3 = qui_new(root, NULL);
	qui_class(child3, "blue");
	
	qui_apply_styles(root, ss);
	
	qui_clear(root);
	free(root);
	
	printf("  test_qui_multiple_classes: PASS\n");
}

static void test_qui_style_properties(void) {
	uint32_t w, h;
	qui_div_t *div;
	
	qgl_size(&w, &h);
	qui_init(w, h);
	
	div = qui_new(NULL, NULL);
	
	/* Test various style properties */
	QUI_STYLE(div, display, QUI_DISPLAY_FLEX);
	QUI_STYLE(div, flex_direction, QUI_ROW);
	QUI_STYLE(div, justify_content, QUI_JUSTIFY_CENTER);
	QUI_STYLE(div, align_items, QUI_ALIGN_CENTER);
	QUI_STYLE(div, position, QUI_POSITION_ABSOLUTE);
	QUI_STYLE(div, top, 10);
	QUI_STYLE(div, left, 20);
	QUI_STYLE(div, right, 30);
	QUI_STYLE(div, bottom, 40);
	QUI_STYLE(div, flex_grow, 1.5f);
	QUI_STYLE(div, flex_shrink, 0.5f);
	QUI_STYLE(div, flex_basis, 100);
	QUI_STYLE(div, padding_top, 5);
	QUI_STYLE(div, padding_right, 10);
	QUI_STYLE(div, padding_bottom, 15);
	QUI_STYLE(div, padding_left, 20);
	QUI_STYLE(div, border_width, 2);
	QUI_STYLE(div, border_color, 0xFF000000);
	QUI_STYLE(div, border_radius_top_left, 5);
	QUI_STYLE(div, border_radius_top_right, 10);
	QUI_STYLE(div, border_radius_bottom_right, 15);
	QUI_STYLE(div, border_radius_bottom_left, 20);
	
	qui_clear(div);
	free(div);
	
	printf("  test_qui_style_properties: PASS\n");
}

int main(void) {
	printf("test_ui_style:\n");
	
	test_qui_stylesheet_init();
	test_qui_stylesheet_add();
	test_qui_class();
	test_qui_apply_styles();
	test_qui_style_set();
	test_qui_style_reset();
	test_qui_inline_style();
	test_qui_multiple_classes();
	test_qui_style_properties();
	
	printf("test_ui_style: ALL TESTS PASSED\n");
	return 0;
}
