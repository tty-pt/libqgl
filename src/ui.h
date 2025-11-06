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

	int needs_layout;

	uint32_t measured_w, measured_h;
	uint32_t measured_cw, measured_ch;
};

void render_div_raw(qui_div_t *d);
void qgl_shadow_margins(const qui_style_t *s,
		int32_t *pl, int32_t *pt,
		int32_t *pr, int32_t *pb);

#endif
