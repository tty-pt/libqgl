#ifndef QGL_UI_INTERNAL_H
#define QGL_UI_INTERNAL_H

#include "../include/ttypt/qgl-ui.h"
#include <stdint.h>

struct qui_div {
	struct qui_div *parent;
	struct qui_div *first_child;
	struct qui_div *next_sibling;
	struct qui_div *last_child;

	int32_t x, y;
	uint32_t w, h, content_w, content_h;

	const char *class_name;
	const char *text;

	qui_style_t *style;
	const char *overflow;

	int style_calloc;
	void *_cache;
};

void render_div_raw(qui_div_t *d);

#endif
