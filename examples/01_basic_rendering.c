/**
 * 01_basic_rendering.c - Basic liblamina rendering example
 * 
 * Demonstrates:
 * - Initializing liblamina and getting screen dimensions
 * - Filling rectangles with colors
 * - Using lam_flush() to present the frame
 * - Basic color format (BGRA)
 * 
 * To compile:
 *   cc -o 01_basic_rendering 01_basic_rendering.c -llamina
 */

#include <ttypt/lam.h>
#include <stdio.h>

int main(void)
{
	uint32_t w, h;
	
	/* Get screen/window dimensions */
	lam_size(&w, &h);
	printf("Window size: %u x %u\n", w, h);
	
	/* Fill background with dark gray (BGRA format: 0xAABBGGRR) */
	lam_fill(0, 0, w, h, 0xFF202020);
	
	/* Draw a red rectangle */
	lam_fill(50, 50, 200, 100, 0xFF0000FF);  /* Red */
	
	/* Draw a green rectangle */
	lam_fill(100, 200, 150, 80, 0xFF00FF00);  /* Green */
	
	/* Draw a blue rectangle */
	lam_fill(300, 150, 180, 120, 0xFFFF0000);  /* Blue */
	
	/* Draw a semi-transparent yellow rectangle */
	lam_fill(200, 300, 250, 100, 0x8000FFFF);  /* Yellow with 50% alpha */
	
	/* Present the frame to the screen */
	lam_flush();
	
	/* Poll for events (keep window open) */
	lam_poll();
	
	return 0;
}
