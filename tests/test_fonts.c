/**
 * test_fonts.c - Font API tests for QGL
 * Tests bitmap font loading, rendering, and text measurement
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ttypt/qgl.h>
#include <ttypt/qgl-font.h>
#include <ttypt/qgl-ui.h>
#include <ttypt/qmap.h>

static void test_font_open(void) {
	uint32_t font_ref;
	
	/* Open font with 8x8 cells, ASCII 32-126 */
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	printf("  test_font_open: PASS (ref=%u)\n", font_ref);
}

static void test_font_draw(void) {
	uint32_t screen_w, screen_h;
	uint32_t font_ref;
	
	qgl_size(&screen_w, &screen_h);
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	qgl_fill(0, 0, screen_w, screen_h, 0xFF000000);
	
	/* Draw simple text */
	const char *text = "Hello World";
	const char *overflow = qgl_font_draw(font_ref, text,
		0, 0, screen_w, screen_h, 1,
		QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Should fit completely */
	assert(overflow == NULL);
	
	qgl_flush();
	
	printf("  test_font_draw: PASS\n");
}

static void test_font_measure(void) {
	uint32_t font_ref;
	uint32_t w = 0, h = 0;
	
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	/* Measure simple text */
	const char *text = "Hello";
	const char *overflow = qgl_font_measure(&w, &h, font_ref, text,
		0, 0, 1000, 1000, 1,
		QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* 5 characters at 8 pixels each = 40 pixels wide, 8 pixels tall */
	assert(w <= 40);  /* Might be less depending on spacing */
	assert(h == 8);
	assert(overflow == NULL);
	
	printf("  test_font_measure: PASS (w=%u, h=%u)\n", w, h);
}

static void test_font_wrapping(void) {
	uint32_t font_ref;
	uint32_t w = 0, h = 0;
	
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	/* Text that should wrap */
	const char *text = "This is a long text that should wrap";
	const char *overflow = qgl_font_measure(&w, &h, font_ref, text,
		0, 0, 100, 100, 1,  /* Narrow width to force wrapping */
		QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Should wrap to multiple lines */
	assert(h > 8);  /* More than one line */
	
	printf("  test_font_wrapping: PASS (h=%u lines)\n", h / 8);
}

static void test_font_overflow(void) {
	uint32_t font_ref;
	uint32_t w = 0, h = 0;
	
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	/* Text that won't fit */
	const char *text = "Very long text that will definitely overflow the tiny box";
	const char *overflow = qgl_font_measure(&w, &h, font_ref, text,
		0, 0, 50, 20, 1,  /* Very small box */
		QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Should have overflow */
	assert(overflow != NULL);
	assert(overflow > text);  /* Points somewhere in the string */
	
	printf("  test_font_overflow: PASS\n");
}

static void test_font_whitespace_modes(void) {
	uint32_t font_ref;
	uint32_t w1 = 0, h1 = 0, w2 = 0, h2 = 0;
	
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	const char *text = "Line1\nLine2";
	
	/* Normal mode - newlines are treated as spaces */
	qgl_font_measure(&w1, &h1, font_ref, text,
		0, 0, 1000, 1000, 1,
		QUI_WS_NORMAL, QUI_WB_NORMAL);
	
	/* Pre mode - newlines create new lines */
	qgl_font_measure(&w2, &h2, font_ref, text,
		0, 0, 1000, 1000, 1,
		QUI_WS_PRE, QUI_WB_NORMAL);
	
	/* Pre mode should be taller (2 lines vs 1) */
	assert(h2 > h1);
	
	printf("  test_font_whitespace_modes: PASS\n");
}

static void test_font_nowrap(void) {
	uint32_t font_ref;
	uint32_t screen_w, screen_h;
	
	qgl_size(&screen_w, &screen_h);
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	qgl_fill(0, 0, screen_w, screen_h, 0xFF000000);
	
	/* Long text with nowrap - should clip */
	const char *text = "This is very long text that should not wrap at all";
	const char *overflow = qgl_font_draw(font_ref, text,
		0, 0, 100, 20, 1,
		QUI_WS_NOWRAP, QUI_WB_NORMAL);
	
	/* Should have overflow because of nowrap */
	assert(overflow != NULL);
	
	qgl_flush();
	
	printf("  test_font_nowrap: PASS\n");
}

static void test_font_scaling(void) {
	uint32_t screen_w, screen_h;
	uint32_t font_ref;
	
	qgl_size(&screen_w, &screen_h);
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	qgl_fill(0, 0, screen_w, screen_h, 0xFF000000);
	
	const char *text = "Scale";
	
	/* Draw at different scales */
	qgl_font_draw(font_ref, text, 0, 0, screen_w, screen_h, 1,
		QUI_WS_NORMAL, QUI_WB_NORMAL);  /* 1x */
	qgl_font_draw(font_ref, text, 0, 20, screen_w, screen_h, 2,
		QUI_WS_NORMAL, QUI_WB_NORMAL);  /* 2x */
	qgl_font_draw(font_ref, text, 0, 50, screen_w, screen_h, 3,
		QUI_WS_NORMAL, QUI_WB_NORMAL);  /* 3x */
	
	qgl_flush();
	
	printf("  test_font_scaling: PASS\n");
}

static void test_font_close(void) {
	uint32_t font_ref;
	
	font_ref = qgl_font_open("tests/fixtures/test_font.png", 8, 8, 32, 126);
	assert(font_ref != QM_MISS);
	
	/* Close should not crash */
	qgl_font_close(font_ref);
	
	/* Closing again should also not crash */
	qgl_font_close(font_ref);
	
	printf("  test_font_close: PASS\n");
}

int main(void) {
	printf("test_fonts:\n");
	
	test_font_open();
	test_font_draw();
	test_font_measure();
	test_font_wrapping();
	test_font_overflow();
	test_font_whitespace_modes();
	test_font_nowrap();
	test_font_scaling();
	test_font_close();
	
	printf("test_fonts: ALL TESTS PASSED\n");
	return 0;
}
