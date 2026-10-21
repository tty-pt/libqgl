/**
 * test_core.c - Core API tests for liblamina
 * Tests basic rendering functions: lam_size, lam_fill, lam_flush, lam_poll
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ttypt/lam.h>

static void test_lam_size(void) {
	uint32_t w = 0, h = 0;
	lam_size(&w, &h);
	
	/* Verify dimensions are reasonable */
	assert(w > 0 && w <= 8192);
	assert(h > 0 && h <= 8192);
	
	printf("  test_lam_size: PASS (w=%u, h=%u)\n", w, h);
}

static void test_lam_fill(void) {
	uint32_t w, h;
	lam_size(&w, &h);
	
	/* Test filling with different colors */
	lam_fill(0, 0, w, h, 0xFF000000);  /* Black */
	lam_fill(0, 0, w/2, h/2, 0xFFFF0000);  /* Red */
	lam_fill(w/2, 0, w/2, h/2, 0xFF00FF00);  /* Green */
	lam_fill(0, h/2, w/2, h/2, 0xFF0000FF);  /* Blue */
	lam_fill(w/2, h/2, w/2, h/2, 0xFFFFFFFF);  /* White */
	
	/* Test edge cases */
	lam_fill(0, 0, 0, 0, 0xFF000000);  /* Zero size */
	lam_fill(w-1, h-1, 1, 1, 0xFFFF00FF);  /* Single pixel */
	
	printf("  test_lam_fill: PASS\n");
}

static void test_lam_flush(void) {
	uint32_t w, h;
	lam_size(&w, &h);
	
	/* Fill and flush */
	lam_fill(0, 0, w, h, 0xFF808080);
	lam_flush();
	
	/* Should not crash on multiple flushes */
	lam_flush();
	lam_flush();
	
	printf("  test_lam_flush: PASS\n");
}

static void test_lam_poll(void) {
	/* Poll should not crash even without user input */
	lam_poll();
	lam_poll();
	lam_poll();
	
	printf("  test_lam_poll: PASS\n");
}

static void test_lam_tint(void) {
	/* Test tint setting */
	lam_tint(0xFFFFFFFF);  /* White (default) */
	lam_tint(0xFF808080);  /* Gray */
	lam_tint(0xFFFF0000);  /* Red tint */
	lam_tint(0x80FFFFFF);  /* Semi-transparent white */
	lam_tint(lam_default_tint);  /* Reset to default */
	
	printf("  test_lam_tint: PASS\n");
}

static void test_multiple_operations(void) {
	uint32_t w, h;
	lam_size(&w, &h);
	
	/* Simulate a typical render loop */
	for (int frame = 0; frame < 3; frame++) {
		lam_fill(0, 0, w, h, 0xFF000000);
		lam_fill(10 * frame, 10 * frame, 50, 50, 0xFFFF0000);
		lam_poll();
		lam_flush();
	}
	
	printf("  test_multiple_operations: PASS\n");
}

int main(void) {
	printf("test_core:\n");
	lam_init();
	test_lam_size();
	test_lam_fill();
	test_lam_flush();
	test_lam_poll();
	test_lam_tint();
	test_multiple_operations();
	
	printf("test_core: ALL TESTS PASSED\n");
	return 0;
}
