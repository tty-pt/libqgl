/**
 * test_textures.c - Texture API tests for QGL
 * Tests texture loading, drawing, tinting, and manipulation
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ttypt/qgl.h>
#include <ttypt/qmap.h>

static void test_tex_load(void) {
	uint32_t tex_ref;
	
	/* Load test texture */
	tex_ref = qgl_tex_load("tests/fixtures/test_texture.png");
	assert(tex_ref != QM_MISS);
	
	/* Load same texture again (should return same ref) */
	uint32_t tex_ref2 = qgl_tex_load("tests/fixtures/test_texture.png");
	assert(tex_ref2 == tex_ref);
	
	/* Load different texture */
	uint32_t tex_small = qgl_tex_load("tests/fixtures/test_small.png");
	assert(tex_small != QM_MISS);
	assert(tex_small != tex_ref);
	
	printf("  test_tex_load: PASS\n");
}

static void test_tex_size(void) {
	uint32_t tex_ref, w = 0, h = 0;
	
	tex_ref = qgl_tex_load("tests/fixtures/test_texture.png");
	assert(tex_ref != QM_MISS);
	
	qgl_tex_size(&w, &h, tex_ref);
	assert(w == 64);
	assert(h == 64);
	
	/* Test small texture */
	tex_ref = qgl_tex_load("tests/fixtures/test_small.png");
	qgl_tex_size(&w, &h, tex_ref);
	assert(w == 16);
	assert(h == 16);
	
	printf("  test_tex_size: PASS (64x64, 16x16)\n");
}

static void test_tex_draw(void) {
	uint32_t screen_w, screen_h;
	uint32_t tex_ref;
	
	qgl_size(&screen_w, &screen_h);
	tex_ref = qgl_tex_load("tests/fixtures/test_texture.png");
	assert(tex_ref != QM_MISS);
	
	/* Clear screen */
	qgl_fill(0, 0, screen_w, screen_h, 0xFF000000);
	
	/* Draw texture at different positions */
	qgl_tex_draw(tex_ref, 0, 0, 64, 64);  /* Original size */
	qgl_tex_draw(tex_ref, 100, 0, 128, 128);  /* Scaled up */
	qgl_tex_draw(tex_ref, 250, 0, 32, 32);  /* Scaled down */
	
	/* Edge cases */
	qgl_tex_draw(tex_ref, 0, 0, 0, 0);  /* Zero size - should not crash */
	qgl_tex_draw(tex_ref, -10, -10, 20, 20);  /* Partially off-screen */
	
	qgl_flush();
	
	printf("  test_tex_draw: PASS\n");
}

static void test_tex_draw_x(void) {
	uint32_t screen_w, screen_h;
	uint32_t tex_ref;
	
	qgl_size(&screen_w, &screen_h);
	tex_ref = qgl_tex_load("tests/fixtures/test_texture.png");
	assert(tex_ref != QM_MISS);
	
	qgl_fill(0, 0, screen_w, screen_h, 0xFF000000);
	
	/* Draw full texture */
	qgl_tex_draw_x(tex_ref, 0, 0, 0, 0, 64, 64, 64, 64, qgl_default_tint);
	
	/* Draw partial texture (top-left quadrant only) */
	qgl_tex_draw_x(tex_ref, 100, 0, 0, 0, 32, 32, 32, 32, qgl_default_tint);
	
	/* Draw bottom-right quadrant */
	qgl_tex_draw_x(tex_ref, 150, 0, 32, 32, 32, 32, 64, 64, qgl_default_tint);
	
	/* Draw with different tints */
	qgl_tex_draw_x(tex_ref, 0, 100, 0, 0, 64, 64, 64, 64, 0xFFFF0000);  /* Red tint */
	qgl_tex_draw_x(tex_ref, 100, 100, 0, 0, 64, 64, 64, 64, 0xFF00FF00);  /* Green tint */
	qgl_tex_draw_x(tex_ref, 200, 100, 0, 0, 64, 64, 64, 64, 0xFF0000FF);  /* Blue tint */
	qgl_tex_draw_x(tex_ref, 300, 100, 0, 0, 64, 64, 64, 64, 0x80FFFFFF);  /* Semi-transparent */
	
	qgl_flush();
	
	printf("  test_tex_draw_x: PASS\n");
}

static void test_tex_tint(void) {
	uint32_t screen_w, screen_h;
	uint32_t tex_ref;
	
	qgl_size(&screen_w, &screen_h);
	tex_ref = qgl_tex_load("tests/fixtures/test_texture.png");
	
	qgl_fill(0, 0, screen_w, screen_h, 0xFF000000);
	
	/* Draw with global tint */
	qgl_tint(0xFFFF0000);  /* Red */
	qgl_tex_draw(tex_ref, 0, 0, 64, 64);
	
	qgl_tint(0xFF00FF00);  /* Green */
	qgl_tex_draw(tex_ref, 70, 0, 64, 64);
	
	qgl_tint(0xFF0000FF);  /* Blue */
	qgl_tex_draw(tex_ref, 140, 0, 64, 64);
	
	/* Reset tint */
	qgl_tint(qgl_default_tint);
	qgl_tex_draw(tex_ref, 210, 0, 64, 64);
	
	qgl_flush();
	
	printf("  test_tex_tint: PASS\n");
}

static void test_tex_pick_paint(void) {
	uint32_t tex_ref;
	uint32_t color;
	
	tex_ref = qgl_tex_load("tests/fixtures/test_texture.png");
	assert(tex_ref != QM_MISS);
	
	/* Pick a color from the texture (top-left should be red) */
	color = qgl_tex_pick(tex_ref, 5, 5);
	/* Note: Color should be 0xFFFF0000 (ARGB) or 0xFF0000FF (ABGR) depending on format */
	assert((color & 0xFF000000) != 0);  /* Has alpha */
	
	/* Paint a pixel */
	qgl_tex_paint(tex_ref, 10, 10, 0xFFFFFFFF);  /* White */
	
	/* Verify the pixel was changed */
	color = qgl_tex_pick(tex_ref, 10, 10);
	assert(color == 0xFFFFFFFF);
	
	printf("  test_tex_pick_paint: PASS\n");
}

static void test_multiple_textures(void) {
	uint32_t screen_w, screen_h;
	uint32_t tex1, tex2;
	
	qgl_size(&screen_w, &screen_h);
	
	tex1 = qgl_tex_load("tests/fixtures/test_texture.png");
	tex2 = qgl_tex_load("tests/fixtures/test_small.png");
	
	assert(tex1 != QM_MISS);
	assert(tex2 != QM_MISS);
	assert(tex1 != tex2);
	
	qgl_fill(0, 0, screen_w, screen_h, 0xFF000000);
	
	/* Draw both textures */
	qgl_tex_draw(tex1, 0, 0, 64, 64);
	qgl_tex_draw(tex2, 70, 0, 64, 64);  /* Scale up small texture */
	qgl_tex_draw(tex1, 140, 0, 32, 32);  /* Scale down large texture */
	qgl_tex_draw(tex2, 180, 0, 16, 16);  /* Original size */
	
	qgl_flush();
	
	printf("  test_multiple_textures: PASS\n");
}

int main(void) {
	printf("test_textures:\n");
	
	test_tex_load();
	test_tex_size();
	test_tex_draw();
	test_tex_draw_x();
	test_tex_tint();
	test_tex_pick_paint();
	test_multiple_textures();
	
	printf("test_textures: ALL TESTS PASSED\n");
	return 0;
}
