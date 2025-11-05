/*
 * qgl-font.c — bitmap-only font loader
 * Loads a pre-rendered tilemap (e.g., from BDF-to-PNG) instead of using FreeType.
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
#include <ttypt/qgl-ui.h>	/* for qui_white_space_t / qui_word_break_t */

/*───────────────────────────────────────────────*
 *                   TYPES                        *
 *───────────────────────────────────────────────*/

struct qgl_glyph {
	uint16_t idx;
};

struct qgl_font_i {
	uint32_t tm_ref;
	uint8_t	first, last;
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
	fprintf(stderr, "font_open ref=%u\n", ref);
	return ref;
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
		qui_white_space_t white_space,
		qui_word_break_t word_break,
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

	const uint32_t cw = scale * tm->w;
	const uint32_t ch = scale * tm->h;

	if (x1 <= x0 || y1 <= y0 || cw == 0 || ch == 0)
		return text;

	uint32_t cx = x0;
	uint32_t cy = y0;
	uint32_t max_w = 0;

	const unsigned char *p = (const unsigned char *)text;

	while (*p) {
		unsigned c = *p;

		/* newline handling */
		if (c == '\n') {
			if (white_space == QUI_WS_PRE ||
					white_space == QUI_WS_PRE_WRAP ||
					white_space == QUI_WS_PRE_LINE) {
				cx = x0;
				cy += ch;
				p++;
				if (cy + ch > y1)
					return (const char *)p;
				continue;
			}
			p++;
			continue;
		}

		/* space handling */
		if (c == ' ') {
			if (white_space == QUI_WS_NORMAL ||
					white_space == QUI_WS_PRE_LINE) {
				/* collapse */
				if (cx != x0)
					cx += cw;
			} else {
				/* preserve */
				cx += cw;
			}
			p++;
			continue;
		}

		/* nowrap → no wrapping */
		if (white_space == QUI_WS_NOWRAP) {
			if (cx + cw > x1)
				break;
		}

		/* compute word length for wrapping */
		const unsigned char *word = p;
		uint32_t word_len = 0;

		while (word[word_len] &&
				word[word_len] != ' ' &&
				word[word_len] != '\n')
			word_len++;

		uint32_t word_px = word_len * cw;

		/* wrap decision */
		if (white_space != QUI_WS_NOWRAP) {
			if (cx != x0 && cx + word_px > x1) {
				switch (word_break) {
					case QUI_WB_BREAK_ALL:
					case QUI_WB_BREAK_WORD: {
									uint32_t room = (x1 - cx) / cw;
									for (uint32_t i = 0; i < room && *p; i++, p++) {
										c = *p;
										if (c >= f->first && c <= f->last && emit)
											emit(user, cx, cy, cw, ch, f->g[c].idx);
										cx += cw;
									}
									cx = x0;
									cy += ch;
									if (cy + ch > y1)
										return (const char *)p;
									continue;
								}
					case QUI_WB_KEEP_ALL:
								cx = x0;
								cy += ch;
								if (cy + ch > y1)
									return (const char *)p;
								break;
					default: /* QUI_WB_NORMAL */
								cx = x0;
								cy += ch;
								if (cy + ch > y1)
									return (const char *)p;
								break;
				}
			}
		}

		/* draw the word */
		for (uint32_t i = 0; i < word_len; i++) {
			c = word[i];
			if (cx + cw > x1 && white_space != QUI_WS_NOWRAP) {
				cx = x0;
				cy += ch;
				if (cy + ch > y1)
					return (const char *)(word + i);
			}

			if (c >= f->first && c <= f->last && emit)
				emit(user, cx, cy, cw, ch, f->g[c].idx);

			cx += cw;
		}

		p += word_len;
		if (cx - x0 > max_w)
			max_w = cx - x0;
	}

	if (max_w_out)
		*max_w_out = max_w;
	if (max_h_out)
		*max_h_out = (cy - y0) + ch;

	return NULL;
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
		qui_white_space_t white_space,
		qui_word_break_t word_break)
{
	struct qgl_font_i *f = get_font(font_ref);
	if (!f)
		return NULL;

	return qgl_font_iterate(font_ref, text,
			x0, y0, x1, y1, scale,
			white_space, word_break,
			font_emit_draw, f, NULL, NULL);
}

const char *qgl_font_measure(uint32_t *w_out, uint32_t *h_out,
		uint32_t font_ref, const char *text,
		uint32_t x0, uint32_t y0,
		uint32_t x1, uint32_t y1,
		uint32_t scale,
		qui_white_space_t white_space,
		qui_word_break_t word_break)
{
	return qgl_font_iterate(font_ref, text,
			x0, y0, x1, y1, scale,
			white_space, word_break,
			NULL, NULL, w_out, h_out);
}
