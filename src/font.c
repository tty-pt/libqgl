/*
 * qgl-font.c — bitmap-only font loader
 * Loads a pre-rendered tilemap (e.g., from BDF-to-PNG) instead of using FreeType.
 *
 * Notes:
 * - Fixed-cell bitmap fonts. Every glyph is one tile in a grid.
 * - We *always* apply the same rounding when scaling (ceilf) so draw/measure match.
 * - We clip per-glyph in qgl_draw_emit() against [x0,x1)×[y0,y1).
 * - Shared core (qgl_font_layout) does both measure and draw; no duplicate logic.
 */

#include "../include/ttypt/qgl.h"
#include "../include/ttypt/qgl-font.h"
#include "../include/ttypt/qgl-tm.h"
#include "../include/ttypt/qgl-ui.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ttypt/qmap.h>
#include <ttypt/qgl-ui.h>	/* qui_white_space_t / qui_word_break_t */

struct qgl_glyph {
	uint16_t idx;
};

struct qgl_font_i {
	uint32_t tm_ref;
	uint8_t	first, last;
	uint16_t cell_w;	/* unscaled cell width (as given on open) */
	uint16_t cell_h;	/* unscaled cell height (as given on open) */
	uint16_t lineh;		/* alias for cell_h for legacy users */
	struct qgl_glyph g[256];
};

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

	font.first  = first;
	font.last   = last;
	font.cell_w = (uint16_t)cell_w;
	font.cell_h = (uint16_t)cell_h;
	font.lineh  = (uint16_t)cell_h;

	/* load atlas and create tilemap */
	{
		uint32_t img_ref = qgl_tex_load(png_path);
		if (!img_ref)
			return QM_MISS;

		uint32_t tm_ref = qgl_tm_new(img_ref, cell_w, cell_h);
		const qgl_tm_t *tm = qgl_tm_get(tm_ref);
		if (!tm)
			return QM_MISS;

		/* fill glyph -> tile index map (grid scanline order) */
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
	}

	/* store font */
	{
		uint32_t ref = qmap_put(g_hd_fonts, NULL, &font);
		fprintf(stderr, "font_open ref=%u\n", ref);
		return ref;
	}
}

void qgl_font_close(uint32_t font_ref)
{
	if (!g_hd_fonts)
		return;
	qmap_del(g_hd_fonts, &font_ref);
}

/*
 * qgl_font_iterate — layout + rendering core
 * Supports white-space and word-break.
 */
static const char *
qgl_font_iterate(uint32_t font_ref, const char *text,
        uint32_t x0, uint32_t y0,
        uint32_t x1, uint32_t y1,
        uint32_t scale,
        qui_white_space_t ws,
        qui_word_break_t wb,
        void (*emit)(void *user, uint32_t x, uint32_t y,
            uint32_t w, uint32_t h, uint16_t glyph_idx),
        void *user,
        uint32_t *max_w_out, uint32_t *max_h_out)
{
	*max_w_out = 0;
	*max_h_out = 0;

	if (!text || !*text)
		return NULL;

	struct qgl_font_i *f = get_font(font_ref);
	if (!f)
		return NULL;

	const qgl_tm_t *tm = qgl_tm_get(f->tm_ref);
	if (!tm)
		return NULL;

	const uint32_t cw = scale * tm->w;
	const uint32_t ch = scale * tm->h;

	if (x1 <= x0 || y1 <= y0 || cw == 0 || ch == 0)
		return text;

	uint32_t cx = x0;
	uint32_t cy = y0;
	uint32_t max_w = 0;
	int in_word = 0;

	const unsigned char *p = (const unsigned char *)text;

	while (*p) {
		unsigned c = *p;

		/* newline */
		if (c == '\n') {
			if (ws == QUI_WS_PRE ||
					ws == QUI_WS_PRE_WRAP ||
					ws == QUI_WS_PRE_LINE)
			{
				cx = x0;
				cy += ch;
			}
			p++;
			continue;
		}

		if (cy + ch > y1)
			break;

		/* spaces */
		if (c == ' ') {
			p++;
			in_word = 0;

			if (cx == x0 &&
					(ws == QUI_WS_NORMAL ||
					 ws == QUI_WS_PRE_LINE))
				continue;

			cx += cw;
			continue;
		}

		/* nowrap */
		if (ws == QUI_WS_NOWRAP) {
			if (cx + cw > x1) {
				p = NULL;
				break;
			}
		}

		/* compute next word length/pixels if at word start */
		uint32_t word_len = 0;
		uint32_t word_px  = 0;

		if (!in_word) {
			const unsigned char *w = p;
			while (w[word_len] &&
					w[word_len] != ' ' &&
					w[word_len] != '\n')
				word_len++;

			word_px = word_len * cw;

			/* simple wrap-at-word-start */
			if (cx + word_px > x1) {
				cx = x0;
				cy += ch;
				continue;
			}
		}

		/* now we are in a word */
		in_word = 1;
		uint32_t room;

		/* advanced word-break logic */
		if (ws != QUI_WS_NOWRAP &&
				cx != x0 &&
				cx + cw > x1)
		{
			switch (wb) {
			case QUI_WB_BREAK_ALL:
			case QUI_WB_BREAK_WORD:
				/* emit as many chars as fit */
				room = (x1 - cx) / cw;
				for (uint32_t i = 0; i < room && *p; i++, p++) {
					c = *p;
					if (c >= f->first && c <= f->last && emit)
						emit(user, cx, cy, cw, ch, f->g[c].idx);
					cx += cw;
				}
				cx = x0;
				cy += ch;

				if (cy + ch > y1)
					break;

				continue;

			case QUI_WB_KEEP_ALL:
				cx = x0;
				cy += ch;
				if (cy + ch > y1)
					break;
				break;

			default: /* QUI_WB_NORMAL */
				cx = x0;
				cy += ch;
				if (cy + ch > y1)
					break;
				break;
			}
		}

		/* emit normal glyph */
		if (c >= f->first && c <= f->last && emit)
			emit(user, cx, cy, cw, ch, f->g[c].idx);

		cx += cw;

		if (cx - x0 > max_w)
			max_w = cx - x0;

		p++;
	}

	*max_w_out = max_w;
	*max_h_out = (cy - y0) + ch;

	return *p ? (const char *)p : NULL;
}

static void font_emit_draw(
		void *user, uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, uint16_t glyph_idx)
{
	const struct qgl_font_i *f = user;
	qgl_tile_draw(f->tm_ref, glyph_idx, x, y, w, h, 1, 1);
}

const char *qgl_font_draw(uint32_t font_ref, const char *text,
		uint32_t x0, uint32_t y0,
		uint32_t x1, uint32_t y1,
		uint32_t scale,
		qui_white_space_t ws,
		qui_word_break_t wb)
{
	struct qgl_font_i *f = get_font(font_ref);
	uint32_t mw, mh;
	if (!f)
		return NULL;

	return qgl_font_iterate(font_ref, text,
			x0, y0, x1, y1, scale,
			ws, wb,
			font_emit_draw, f, &mw, &mh);
}

const char *qgl_font_measure(uint32_t *w_out, uint32_t *h_out,
		uint32_t font_ref, const char *text,
		uint32_t x0, uint32_t y0,
		uint32_t x1, uint32_t y1,
		uint32_t scale,
		qui_white_space_t ws,
		qui_word_break_t wb)
{
	return qgl_font_iterate(font_ref, text,
			x0, y0, x1, y1, scale,
			ws, wb,
			NULL, NULL, w_out, h_out);
}
