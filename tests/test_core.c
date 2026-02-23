/**
 * test_core.c - Core API tests for QGL
 * Tests basic rendering functions: qgl_size, qgl_fill, qgl_flush, qgl_poll
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ttypt/qgl.h>

static void test_qgl_size(void) {
	uint32_t w = 0, h = 0;
	qgl_size(&w, &h);
	
	/* Verify dimensions are reasonable */
	assert(w > 0 && w <= 8192);
	assert(h > 0 && h <= 8192);
	
	printf("  test_qgl_size: PASS (w=%u, h=%u)\n", w, h);
}

static void test_qgl_fill(void) {
	uint32_t w, h;
	qgl_size(&w, &h);
	
	/* Test filling with different colors */
	qgl_fill(0, 0, w, h, 0xFF000000);  /* Black */
	qgl_fill(0, 0, w/2, h/2, 0xFFFF0000);  /* Red */
	qgl_fill(w/2, 0, w/2, h/2, 0xFF00FF00);  /* Green */
	qgl_fill(0, h/2, w/2, h/2, 0xFF0000FF);  /* Blue */
	qgl_fill(w/2, h/2, w/2, h/2, 0xFFFFFFFF);  /* White */
	
	/* Test edge cases */
	qgl_fill(0, 0, 0, 0, 0xFF000000);  /* Zero size */
	qgl_fill(w-1, h-1, 1, 1, 0xFFFF00FF);  /* Single pixel */
	
	printf("  test_qgl_fill: PASS\n");
}

static void test_qgl_flush(void) {
	uint32_t w, h;
	qgl_size(&w, &h);
	
	/* Fill and flush */
	qgl_fill(0, 0, w, h, 0xFF808080);
	qgl_flush();
	
	/* Should not crash on multiple flushes */
	qgl_flush();
	qgl_flush();
	
	printf("  test_qgl_flush: PASS\n");
}

static void test_qgl_poll(void) {
	/* Poll should not crash even without user input */
	qgl_poll();
	qgl_poll();
	qgl_poll();
	
	printf("  test_qgl_poll: PASS\n");
}

static void test_qgl_tint(void) {
	/* Test tint setting */
	qgl_tint(0xFFFFFFFF);  /* White (default) */
	qgl_tint(0xFF808080);  /* Gray */
	qgl_tint(0xFFFF0000);  /* Red tint */
	qgl_tint(0x80FFFFFF);  /* Semi-transparent white */
	qgl_tint(qgl_default_tint);  /* Reset to default */
	
	printf("  test_qgl_tint: PASS\n");
}

static void test_multiple_operations(void) {
	uint32_t w, h;
	qgl_size(&w, &h);
	
	/* Simulate a typical render loop */
	for (int frame = 0; frame < 3; frame++) {
		qgl_fill(0, 0, w, h, 0xFF000000);
		qgl_fill(10 * frame, 10 * frame, 50, 50, 0xFFFF0000);
		qgl_poll();
		qgl_flush();
	}
	
	printf("  test_multiple_operations: PASS\n");
}

int main(void) {
	printf("test_core:\n");
	
	test_qgl_size();
	test_qgl_fill();
	test_qgl_flush();
	test_qgl_poll();
	test_qgl_tint();
	test_multiple_operations();
	
	printf("test_core: ALL TESTS PASSED\n");
	return 0;
}
