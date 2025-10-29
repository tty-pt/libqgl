/*
 * This file contains code under a dual licensing scheme:
 * - Portions © Pedro Tavares, licensed under Apache 2.0 (see NOTICE)
 * - Remaining portions © 2025 Paulo André Azevedo Quirino, under BSD-2-Clause
 */

#include "../include/ttypt/qgl.h"

#include <stddef.h>

#include "be.h"

screen_t screen;
uint32_t qgl_width, qgl_height;

void be_render(qgl_lambda_t *lambda,
		int32_t x, int32_t y,
		uint32_t w, uint32_t h, void *ctx)
{
	uint32_t sw = qgl_width;
	uint8_t channels = screen.channels;
	size_t offset = y * sw + x;
	uint8_t *start = &screen.canvas[0]
		+ offset * channels;
	uint8_t *pos = start;

	if (x + w > qgl_width)
		w = qgl_width - x;

	if (y + h > qgl_height)
		h = qgl_height - y;

	if (x < (int32_t) screen.min_x)
		screen.min_x = x;

	if (y < (int32_t) screen.min_y)
		screen.min_y = y;

	if (x + w > screen.max_x)
		screen.max_x = x + w;

	if (y + h > screen.max_y)
		screen.max_y = y + h;

	for (
			uint32_t kce = (y + h) * sw,
			kc = offset, kcm = kc + w;

			kc < kce;

			kc ++, pos += channels)
	{
		if (kc > kcm) {
			kc += sw - w - 1;
			if (kc >= kce)
				break;
			kcm = kc + w;
			pos = &screen.canvas[0]
				+ kc * channels;
		}

		uint32_t i = kc / sw;
		uint32_t j = kc % sw;
		uint32_t ix = j - x;
		lambda(pos, ix, i - y, ctx);
	}
}
