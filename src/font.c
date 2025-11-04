/*
 * qgl-font.c — bitmap-only font loader
 * Loads a pre-rendered tilemap (e.g., from BDF-to-PNG) instead of using FreeType.
 */

#include "../include/ttypt/qgl.h"
#include "../include/ttypt/qgl-font.h"
#include "../include/ttypt/qgl-tm.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ttypt/qmap.h>

/*───────────────────────────────────────────────*
 *                   TYPES                        *
 *───────────────────────────────────────────────*/

struct qgl_glyph {
	uint16_t idx;    /* tile index inside tilemap */
};

struct qgl_font_i {
	uint32_t tm_ref; /* tilemap handle */
	uint8_t  first, last;
	uint16_t lineh;
	struct qgl_glyph g[256];
};

/*───────────────────────────────────────────────*
 *                INTERNAL STATE                  *
 *───────────────────────────────────────────────*/

static uint32_t g_hd_fonts;
static uint32_t g_type_font;

static int ensure_maps(void)
{
	if (g_hd_fonts)
		return 0;

	g_type_font = qmap_reg(sizeof(struct qgl_font_i));
	g_hd_fonts = qmap_open(NULL, NULL,
	                       QM_HNDL, g_type_font,
	                       (1u << 8) - 1, QM_AINDEX);
	return g_hd_fonts ? 0 : -1;
}

static inline struct qgl_font_i *get_font(uint32_t ref)
{
	const void *v = qmap_get(g_hd_fonts, &ref);
	return (struct qgl_font_i *)v;
}

/*───────────────────────────────────────────────*
 *                    API                         *
 *───────────────────────────────────────────────*/

/**
 * qgl_font_open_bitmap
 * @brief Load a pre-rendered bitmap font atlas.
 *
 * @param png_path path to PNG tilemap
 * @param cell_w   width of each cell in pixels
 * @param cell_h   height of each cell in pixels
 * @param first    first ASCII code represented
 * @param last     last ASCII code represented
 *
 * @return qmap handle reference, or 0 on error
 */
uint32_t qgl_font_open(const char *png_path,
                       unsigned cell_w,
                       unsigned cell_h,
                       uint8_t first,
                       uint8_t last)
{
	if (!png_path || cell_w == 0 || cell_h == 0)
		return QM_MISS;
	if (ensure_maps() != 0)
		return QM_MISS;

	struct qgl_font_i font;
	memset(&font, 0, sizeof(font));

	font.first = first;
	font.last  = last;
	font.lineh = cell_h;

	uint32_t img_ref = qgl_tex_load(png_path);
	if (!img_ref)
		return QM_MISS;

	uint32_t tm_ref = qgl_tm_new(img_ref, cell_w, cell_h);
	const qgl_tm_t *tm = qgl_tm_get(tm_ref);
	if (!tm)
		return QM_MISS;

	unsigned gx = 0, gy = 0;
	for (int c = font.first; c <= font.last; c++) {
		uint16_t idx = (uint16_t)(gy * tm->nx + gx);
		font.g[c].idx = idx;

		if (++gx == tm->nx) {
			gx = 0;
			gy++;
		}
	}

	font.tm_ref = tm_ref;

	uint32_t ref = qmap_put(g_hd_fonts, NULL, &font);
	fprintf(stderr, "font_open4 ref=%u\n", ref);
	return ref;
}

void qgl_font_close(uint32_t font_ref)
{
	if (!g_hd_fonts)
		return;
	qmap_del(g_hd_fonts, &font_ref);
}

static const char *
qgl_font_iterate(uint32_t font_ref, const char *text,
		 uint32_t x0, uint32_t y0,
		 uint32_t x1, uint32_t y1,
		 uint32_t scale,
		 void (*emit)(void *user, uint32_t x, uint32_t y,
			      uint32_t w, uint32_t h, uint16_t glyph_idx),
		 void *user,
		 uint32_t *max_w_out, uint32_t *max_h_out)
{
	if (max_w_out)
		*max_w_out = 0;
	if (max_h_out)
		*max_h_out = 0;

	if (!text || !*text)
		return NULL;

	struct qgl_font_i *f = get_font(font_ref);
	if (!f)
		return NULL;

	const qgl_tm_t *tm = qgl_tm_get(f->tm_ref);
	if (!tm)
		return NULL;

	const uint32_t cell_w = scale * tm->w;
	const uint32_t cell_h = scale * tm->h;

	if (x1 <= x0 || y1 <= y0 || cell_w == 0 || cell_h == 0)
		return text;

	uint32_t cx = x0;
	uint32_t cy = y0;
	uint32_t max_w = 0;

	for (const unsigned char *p = (const unsigned char *)text; *p; ++p) {
		const unsigned c = *p;

		if (cx + cell_w > x1) {
			if (cx - x0 > max_w)
				max_w = cx - x0;

			cy += cell_h;
			cx = x0;
		}

		if (cy + cell_h > y1)
			return (const char *) p;

		if (c == '\n') {
			cx = x0;
			cy += cell_h;
			continue;
		}

		if (c >= f->first && c <= f->last && emit)
			emit(user, cx, cy, cell_w, cell_h, f->g[c].idx);

		cx += cell_w;
	}

	if (cx - x0 > max_w)
		max_w = cx - x0;

	if (max_w_out)
		*max_w_out = max_w;
	if (max_h_out)
		*max_h_out = (cy - y0) + cell_h;

	return NULL;
}

static void font_emit_draw(
		void *user, uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, uint16_t glyph_idx)
{
	const struct qgl_font_i *f = user;
	qgl_tile_draw(f->tm_ref, glyph_idx, x, y, w, h, 1, 1);
}

const char *qgl_font_draw(
		uint32_t font_ref,
		const char *text,
		uint32_t x0, uint32_t y0,
		uint32_t x1, uint32_t y1,
		uint32_t scale)
{
	struct qgl_font_i *f = get_font(font_ref);

	if (!f)
		return NULL;

	return qgl_font_iterate(font_ref, text,
			x0, y0, x1, y1, scale,
			font_emit_draw, f, NULL, NULL);
}

const char *qgl_font_measure(
		uint32_t *w_out, uint32_t *h_out,
		uint32_t font_ref, const char *text,
		uint32_t x0, uint32_t y0,
		uint32_t x1, uint32_t y1,
		uint32_t scale)
{
	return qgl_font_iterate(font_ref, text,
			x0, y0, x1, y1, scale,
			NULL, NULL, w_out, h_out);
}
