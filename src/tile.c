#include "../include/ttypt/qgl.h"
#include "../include/ttypt/qgl-tm.h"

#include <stdio.h>
#include <string.h>

#include <ttypt/qmap.h>
#include <ttypt/qsys.h>

static uint32_t tm_hd;

uint32_t
qgl_tm_new(uint32_t img_ref, uint32_t w, uint32_t h)
{
	qgl_tm_t tm;
	uint32_t img_w, img_h;

	tm.img = img_ref;
	qgl_tex_size(&img_w, &img_h, img_ref);
	tm.w = w;
	tm.h = h;
	tm.nx = img_w / w;
	tm.ny = img_h / h;

	uint32_t ref = qmap_put(tm_hd, NULL, &tm);
	WARN("tm_load %u: %u %u %u\n", ref, img_ref,
			w, h);
	return ref;
}

void
qgl_tile_draw(uint32_t ref, uint32_t idx,
		uint32_t x, uint32_t y,
		uint32_t w, uint32_t h,
		uint32_t rx, uint32_t ry)
{
	const qgl_tm_t *tm = qmap_get(tm_hd, &ref);

	unsigned tm_x = idx % tm->nx;
	unsigned tm_y = idx / tm->nx;

	if (!w)
		w = tm->w;
	if (!h)
		h = tm->h;

	for (uint32_t iy = 0; iy < ry * h; iy += h)
		for (uint32_t ix = 0; ix < rx * w; ix += w)
			qgl_tex_draw_x(tm->img,
					x + ix,
					y + iy,
					tm_x * tm->w,
					tm_y * tm->h,
					tm->w, tm->h,
					w, h, qgl_default_tint);
}

const qgl_tm_t *
qgl_tm_get(uint32_t ref)
{
	return qmap_get(tm_hd, &ref);
}

void __attribute__((constructor))
constructor(void)
{
	uint32_t qm_tm = qmap_reg(sizeof(qgl_tm_t));

	tm_hd = qmap_open(NULL, NULL, QM_HNDL, qm_tm, 0xF, QM_AINDEX);
}
