/**
 * 01_basic_rendering.c - Basic QGL rendering example
 * 
 * Demonstrates:
 * - Initializing QGL and getting screen dimensions
 * - Filling rectangles with colors
 * - Using qgl_flush() to present the frame
 * - Basic color format (BGRA)
 * 
 * To compile:
 *   cc -o 01_basic_rendering 01_basic_rendering.c -lqgl
 */

#include <ttypt/qgl.h>
#include <stdio.h>

int main(void)
{
	uint32_t w, h;
	
	/* Get screen/window dimensions */
	qgl_size(&w, &h);
	printf("Window size: %u x %u\n", w, h);
	
	/* Fill background with dark gray (BGRA format: 0xAABBGGRR) */
	qgl_fill(0, 0, w, h, 0xFF202020);
	
	/* Draw a red rectangle */
	qgl_fill(50, 50, 200, 100, 0xFF0000FF);  /* Red */
	
	/* Draw a green rectangle */
	qgl_fill(100, 200, 150, 80, 0xFF00FF00);  /* Green */
	
	/* Draw a blue rectangle */
	qgl_fill(300, 150, 180, 120, 0xFFFF0000);  /* Blue */
	
	/* Draw a semi-transparent yellow rectangle */
	qgl_fill(200, 300, 250, 100, 0x8000FFFF);  /* Yellow with 50% alpha */
	
	/* Present the frame to the screen */
	qgl_flush();
	
	/* Poll for events (keep window open) */
	qgl_poll();
	
	return 0;
}
