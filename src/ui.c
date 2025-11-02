#include "../include/ttypt/qgl-ui.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <ttypt/qmap.h>

struct qui_div {
	struct qui_div *parent;
	struct qui_div *first_child;
	struct qui_div *next_sibling;

	int32_t x, y;
	uint32_t w, h, content_w, content_h;

	const char *class_name;
	const char *text;

	qui_style_t *style;
	const char *overflow;

	int style_calloc;
};

static uint32_t g_screen_w, g_screen_h;

static void qui_style_reset(qui_style_t *s)
{
	memset(s, 0, sizeof(*s));
	s->font_scale = 1;
	s->display = UI_DISPLAY_BLOCK;
	s->position = UI_POS_RELATIVE;
	s->dir = UI_COLUMN;
	s->align_items = UI_ALIGN_STRETCH;
	s->justify_content = UI_JUSTIFY_START;

	s->grow = 1.0f;      /* <— novo: ocupa espaço livre no eixo principal */
	s->shrink = 0;
	s->basis = 1;        /* auto */
}

static void qui_style_merge(qui_style_t *dst, const qui_style_t *src)
{
	if (!src)
		return;

#define MERGE(f)			\
	do {				\
		if (src->f)		\
		dst->f = src->f;	\
	} while (0)

	MERGE(bg_color);
	MERGE(border_color);
	MERGE(border_size);
	MERGE(bg_tex);
	if (src->font_ref != QM_MISS)
		dst->font_ref = src->font_ref;
	if (src->font_scale != 1)
		dst->font_scale = src->font_scale;
	MERGE(text_color);
	MERGE(align);

	MERGE(pad_left);
	MERGE(pad_right);
	MERGE(pad_top);
	MERGE(pad_bottom);

	MERGE(position);
	MERGE(left);
	MERGE(right);
	MERGE(top);
	MERGE(bottom);

	dst->display = src->display;

	/* We must be able to merge 0 values for flex properties */
	if (src->grow != 1.0f) /* Check grow, but allow 0 to be merged if set */
		dst->grow = src->grow;
	if (src->shrink != 0.0f)
		dst->shrink = src->shrink;
	if (src->basis != 1) /* '1' is the default from reset */
		dst->basis = src->basis;

	dst->dir = src->dir;

	MERGE(align_items);
	MERGE(justify_content);
#undef MERGE
}

void qui_stylesheet_init(qui_style_rule_t **rules)
{
	*rules = NULL;
}

void qui_stylesheet_add(qui_style_rule_t **ss,
		const char *class_name,
		const qui_style_t *style)
{
	qui_style_rule_t *r = calloc(1, sizeof(*r));

	if (!r)
		return;

	r->class_name = class_name;
	r->style = *style;
	r->next = *ss;
	*ss = r;
}

static const qui_style_t *qui_stylesheet_lookup(qui_style_rule_t *ss,
		const char *class_name)
{
	qui_style_rule_t *r;

	for (r = ss; r; r = r->next)
		if (r->class_name && class_name &&
				strcmp(r->class_name, class_name) == 0)
			return &r->style;

	return NULL;
}

void qui_init(uint32_t screen_w, uint32_t screen_h)
{
	g_screen_w = screen_w;
	g_screen_h = screen_h;
	(void)g_screen_w;
	(void)g_screen_h;
}

static void qui_mark_dirty(qui_div_t *d)
{
    if (!d) return;
    d->w = d->h = 0;
    for (qui_div_t *c = d->first_child; c; c = c->next_sibling)
        qui_mark_dirty(c);
}

qui_div_t *qui_new(qui_div_t *parent, qui_style_t *style)
{
	qui_div_t *d = calloc(1, sizeof(*d));
	qui_div_t *c;

	if (!d)
		return NULL;

	if (style) {
		d->style = style;
		d->style_calloc = 0;
	} else {
		d->style = calloc(1, sizeof(qui_style_t));
		d->style_calloc = 1;
		qui_style_reset(d->style);
	}

	if (parent) {
		d->parent = parent;

		if (!parent->first_child) {
			parent->first_child = d;
		} else {
			c = parent->first_child;
			while (c->next_sibling)
				c = c->next_sibling;
			c->next_sibling = d;
		}

		qui_mark_dirty(parent);
	}

	return d;
}

void qui_class(qui_div_t *div, const char *class_name)
{
	div->class_name = class_name;
}

void qui_text(qui_div_t *div, const char *text)
{
	div->text = text;
	qui_mark_dirty(div);
}

static void apply_style_recursive(qui_div_t *div,
		qui_style_rule_t *ss,
		const qui_style_t *inherited)
{
	const qui_style_t *class_style;
	qui_style_t merged = *inherited;
	qui_div_t *c;

	/* 1) Cascade: inherited → class → inline(decl) */
	class_style = qui_stylesheet_lookup(ss, div->class_name);
	qui_style_merge(&merged, class_style);
	if (!div->style_calloc)
		qui_style_merge(&merged, div->style);

	/* 2) Aplicar ao nó SEM tocar no estilo partilhado */
	if (div->style_calloc)
		*div->style = merged;
	else
		qui_style_merge(div->style, &merged);

	/* 3) Herança para filhos: não propagar absolutos/decoração;
	   NÃO tocar em grow/shrink/basis */
	qui_style_t child_inh = *div->style;

	child_inh.position = UI_POS_RELATIVE;
	child_inh.left = child_inh.right = 0;
	child_inh.top  = child_inh.bottom = 0;

	child_inh.pad_top = child_inh.pad_bottom = 0;
	child_inh.pad_left = child_inh.pad_right = 0;
	child_inh.border_color = 0;
	child_inh.border_size = 0;
	child_inh.bg_color = 0;
	child_inh.bg_tex = 0;
	child_inh.grow = 0;
	child_inh.shrink = 0;
	child_inh.basis = 1;
	child_inh.dir = UI_COLUMN;

	/* manter child_inh.grow/shrink/basis tal como estão (do reset/declaração) */

	for (c = div->first_child; c; c = c->next_sibling)
		apply_style_recursive(c, ss, &child_inh);
}

void qui_apply_styles(qui_div_t *root, qui_style_rule_t *ss)
{
	qui_style_t base;

	qui_style_reset(&base);
	apply_style_recursive(root, ss, &base);
}

static void measure_content_size(qui_div_t *c)
{
	uint32_t inner_w = 0, inner_h = 0;

	for (qui_div_t *ch = c->first_child; ch; ch = ch->next_sibling) {
		if (ch->style->display == UI_DISPLAY_NONE)
			continue;
		measure_content_size(ch);
		if (c->style->dir == UI_ROW) {
			inner_w += ch->w;
			if (ch->h > inner_h)
				inner_h = ch->h;
		} else {
			inner_h += ch->h;
			if (ch->w > inner_w)
				inner_w = ch->w;
		}
	}

	/* texto */
	if (!c->first_child && c->text && c->style->font_ref != QM_MISS) {
		uint32_t tw = 0, th = 0;
		qgl_font_measure(&tw, &th,
				 c->style->font_ref, c->text,
				 0, 0, UINT32_MAX, UINT32_MAX,
				 c->style->font_scale);
		if (!inner_w)
			inner_w = tw;
		if (!inner_h)
			inner_h = th;
	}

	/* If no content and no explicit size, use basis as initial size for flex items */
	/* If no content and no explicit size, use basis as initial size for flex items */
if (c->style->grow > 0 && c->style->basis > 0) {
	/*
	 * Apply basis as fallback size for both axes.
	 * The previous 'else if' made empty siblings report
	 * different initial sizes depending on their 'dir',
	 * which broke perfect 50/50 vertical splits.
	 */
	if (inner_w == 0)
		inner_w = c->style->basis;
	if (inner_h == 0)
		inner_h = c->style->basis;
}

	/* padding */
	inner_w += c->style->pad_left + c->style->pad_right;
	inner_h += c->style->pad_top + c->style->pad_bottom;

	if (!c->w)
		c->w = inner_w;
	if (!c->h)
		c->h = inner_h;

	c->content_w = inner_w;
	c->content_h = inner_h;
}

static void measure_text_overflow(qui_div_t *c)
{
	c->overflow = NULL;

	if (!c->text || c->style->font_ref == QM_MISS)
		return;

	int32_t tx = c->x + (int32_t)c->style->pad_left;
	int32_t ty = c->y + (int32_t)c->style->pad_top;

	int32_t tw = (int32_t)c->w - (int32_t)(c->style->pad_left + c->style->pad_right);
	int32_t th = (int32_t)c->h - (int32_t)(c->style->pad_top + c->style->pad_bottom);

	if (tw <= 0 || th <= 0) {
		c->overflow = c->text;
		return;
	}

	/* usar clip e confiar no retorno (primeiro char não desenhado) */
	uint32_t mw = 0, mh = 0;
	const char *ov = qgl_font_measure(&mw, &mh,
	                                  c->style->font_ref, c->text,
	                                  tx, ty, tx + tw, ty + th,
	                                  c->style->font_scale);

	c->overflow = ov; /* NULL = coube tudo; não-NULL = sobrou texto */
}

/*───────────────────────────────*
 *   Helpers de layout interno   *
 *───────────────────────────────*/

static void layout_absolute_child(qui_div_t *p, qui_div_t *c)
{
	/* horizontais */
	int have_left  = (c->style->left  != 0);
	int have_right = (c->style->right != 0);

	if (have_left && have_right) {
		c->x = p->x + c->style->left;
		c->w = p->w - c->style->left - c->style->right;
	} else if (have_left) {
		c->x = p->x + c->style->left;
		/* c->w fica como estiver (pode vir medido/definido) */
	} else if (have_right) {
		c->x = p->x + (int32_t)p->w - (int32_t)c->w - c->style->right;
	} else {
		/* nenhum definido: ocupar tudo */
		c->x = p->x;
		c->w = p->w;
	}

	/* verticais */
	int have_top    = (c->style->top    != 0);
	int have_bottom = (c->style->bottom != 0);

	if (have_top && have_bottom) {
		c->y = p->y + c->style->top;
		c->h = p->h - c->style->top - c->style->bottom;
	} else if (have_top) {
		c->y = p->y + c->style->top;
	} else if (have_bottom) {
		c->y = p->y + (int32_t)p->h - (int32_t)c->h - c->style->bottom;
	} else {
		/* nenhum definido: ocupar tudo */
		c->y = p->y;
		c->h = p->h;
	}
}

static void layout_compute_totals(qui_div_t *p,
		uint32_t *count_r,
		uint32_t *total_main_r,
		uint32_t *max_cross_r)
{
	uint32_t count = 0, total_main = 0, max_cross = 0;

	for (qui_div_t *c = p->first_child; c; c = c->next_sibling) {
		if (c->style->display == UI_DISPLAY_NONE ||
				c->style->position != UI_POS_RELATIVE)
			continue;
		count++;

		if (p->style->dir == UI_ROW) {
			total_main += c->w;
			if (c->h > max_cross)
				max_cross = c->h;
		} else {
			total_main += c->h;
			if (c->w > max_cross)
				max_cross = c->w;
		}
	}

	*count_r = count;
	*total_main_r = total_main;
	*max_cross_r = max_cross;
}

static void layout_apply_flex_grow(qui_div_t *p,
		uint32_t inner_w,
		uint32_t inner_h,
		uint32_t *total_main_r,
		uint32_t *max_cross_r)
{
	float total_grow = 0.0f;
	unsigned eligible = 0;
	int32_t container_main =
		(p->style->dir == UI_ROW) ? (int32_t)inner_w : (int32_t)inner_h;

	/* Pass 1: collect total grow and eligible children */
	for (qui_div_t *c = p->first_child; c; c = c->next_sibling) {
		if (c->style->display == UI_DISPLAY_NONE ||
		    c->style->position != UI_POS_RELATIVE)
			continue;
		if (c->style->grow > 0.0f) {
			total_grow += c->style->grow;
			eligible++;
		}
	}

	if (total_grow <= 0.0f || eligible == 0)
		return;

	int32_t total_main = (int32_t)(*total_main_r);
	int32_t remaining = container_main - total_main;
	if (remaining < 0)
		remaining = 0;

	/* Redistribute proportionally by grow */
	float acc = 0.0f;
	unsigned assigned = 0;

	for (qui_div_t *c = p->first_child; c; c = c->next_sibling) {
		if (c->style->display == UI_DISPLAY_NONE ||
		    c->style->position != UI_POS_RELATIVE ||
		    c->style->grow <= 0.0f)
			continue;

		assigned++;

		float ratio = c->style->grow / total_grow;
		int32_t share;

		if (assigned == eligible) {
			/* last child gets remaining pixels */
			share = container_main - (int32_t)acc;
		} else {
			share = (int32_t)((float)container_main * ratio);
			acc += (float)share;
		}

		if (share < 0)
			share = 0;

		if (p->style->dir == UI_ROW)
			c->w = (uint32_t)share;
		else
			c->h = (uint32_t)share;
	}

	/* Recompute totals after flex grow */
	uint32_t dummy = 0;
	layout_compute_totals(p, &dummy, total_main_r, max_cross_r);
}


static void layout_children(qui_div_t *p);

static void layout_compute_justify(const qui_div_t *p,
		uint32_t count,
		uint32_t total_main,
		uint32_t inner_w,
		uint32_t inner_h,
		int32_t *start_offset_r,
		int32_t *spacing_r)
{
	int32_t container_main = (p->style->dir == UI_ROW)
		? (int32_t)inner_w : (int32_t)inner_h;
	int32_t remaining = container_main - (int32_t)total_main;
	if (remaining < 0)
		remaining = 0;

	int32_t start_offset = 0;
	int32_t spacing = 0;

	switch (p->style->justify_content) {
		case UI_JUSTIFY_CENTER:
			start_offset = remaining / 2;
			break;
		case UI_JUSTIFY_END:
			start_offset = remaining;
			break;
		case UI_JUSTIFY_SPACE_BETWEEN:
			spacing = (count > 1) ? remaining / (int32_t)(count - 1) : 0;
			break;
		case UI_JUSTIFY_SPACE_AROUND:
			spacing = (count > 0) ? remaining / (int32_t)count : 0;
			start_offset = spacing / 2;
			break;
		default:
			break;
	}

	*start_offset_r = start_offset;
	*spacing_r = spacing;
}

static void layout_place_children(qui_div_t *p,
		int32_t inner_x,
		int32_t inner_y,
		uint32_t inner_w,
		uint32_t inner_h,
		int32_t start_offset,
		int32_t spacing)
{
	int32_t cur_x = inner_x;
	int32_t cur_y = inner_y;

	if (p->style->dir == UI_ROW)
		cur_x += start_offset;
	else
		cur_y += start_offset;

	for (qui_div_t *c = p->first_child; c; c = c->next_sibling) {
		if (c->style->display == UI_DISPLAY_NONE)
			continue;

		if (c->style->position == UI_POS_RELATIVE) {
			if (p->style->dir == UI_ROW) {
				/* eixo principal: x */
				c->x = cur_x;

				/* eixo cruzado (altura) */
				switch (p->style->align_items) {
					case UI_ALIGN_START:
						c->y = inner_y;
						break;
					case UI_ALIGN_CENTER:
						c->y = inner_y + (int32_t)(inner_h - c->h) / 2;
						break;
					case UI_ALIGN_END:
						c->y = inner_y + (int32_t)(inner_h - c->h);
						break;
					case UI_ALIGN_STRETCH: /* default */
					default:
						c->y = inner_y;
						c->h = inner_h;
						break;
				}

				/* NENHUM CLAMP NO EIXO PRINCIPAL.
				 * Mantemos c->w tal como medido/grow/basis. */

				/* layout interno e overflow com tamanho final */
				layout_children(c);
				measure_text_overflow(c);

				cur_x += (int32_t)c->w + spacing;

			} else {
				/* UI_COLUMN: eixo principal: y */
				c->y = cur_y;

				/* eixo cruzado (largura) */
				switch (p->style->align_items) {
					case UI_ALIGN_START:
						c->x = inner_x;
						break;
					case UI_ALIGN_CENTER:
						c->x = inner_x + (int32_t)(inner_w - c->w) / 2;
						break;
					case UI_ALIGN_END:
						c->x = inner_x + (int32_t)(inner_w - c->w);
						break;
					case UI_ALIGN_STRETCH: /* default */
					default:
						c->x = inner_x;
						c->w = inner_w;
						break;
				}

				/* NENHUM CLAMP NO EIXO PRINCIPAL. */

				/* layout interno e overflow com tamanho final */
				layout_children(c);
				measure_text_overflow(c);

				cur_y += (int32_t)c->h + spacing;
			}
		} else {
			/* absolute: posiciona e mede com a caixa final */
			layout_absolute_child(p, c);
			layout_children(c);
			measure_text_overflow(c);
		}
	}
}

static void layout_children(qui_div_t *p)
{
	if (!p->first_child)
		return;

	/* First pass: measure content with initial sizes */
	for (qui_div_t *c = p->first_child; c; c = c->next_sibling) {
		if (c->style->display != UI_DISPLAY_NONE)
			measure_content_size(c);
	}

	/* Calculate available space and apply flex grow */
	uint32_t count, total_main, max_cross;
	layout_compute_totals(p, &count, &total_main, &max_cross);

	int32_t inner_x = p->x + (int32_t)p->style->pad_left;
	int32_t inner_y = p->y + (int32_t)p->style->pad_top;
	uint32_t inner_w = p->w - (p->style->pad_left + p->style->pad_right);
	uint32_t inner_h = p->h - (p->style->pad_top  + p->style->pad_bottom);

	layout_apply_flex_grow(p, inner_w, inner_h, &total_main, &max_cross);

	/* Second pass: re-measure content with final flex sizes */
	for (qui_div_t *c = p->first_child; c; c = c->next_sibling) {
		if (c->style->display != UI_DISPLAY_NONE) {
			/* Reset content measurements to force re-calculation */
			c->content_w = 0;
			c->content_h = 0;
			measure_content_size(c);
		}
	}

	/* Recalculate totals after final measurement */
	layout_compute_totals(p, &count, &total_main, &max_cross);

	int32_t start_offset, spacing;
	layout_compute_justify(p, count, total_main, inner_w, inner_h,
			       &start_offset, &spacing);

	layout_place_children(p, inner_x, inner_y, inner_w, inner_h,
			      start_offset, spacing);

	/* Final content size for parent */
	if (p->style->dir == UI_ROW) {
		p->content_w = total_main;
		p->content_h = max_cross;
	} else {
		p->content_w = max_cross;
		p->content_h = total_main;
	}

	if (p->style->display == UI_DISPLAY_INLINE) {
		if (p->style->dir == UI_ROW) {
			p->w = p->content_w + p->style->pad_left + p->style->pad_right;
			if (!p->h)
				p->h = p->content_h + p->style->pad_top + p->style->pad_bottom;
		} else {
			p->h = p->content_h + p->style->pad_top + p->style->pad_bottom;
			if (!p->w)
				p->w = p->content_w + p->style->pad_left + p->style->pad_right;
		}
	}

	measure_text_overflow(p);
}

void qui_layout(qui_div_t *root, int32_t x, int32_t y,
		uint32_t w, uint32_t h)
{
	root->x = x;
	root->y = y;
	root->w = w;
	root->h = h;
	root->style->display = UI_DISPLAY_BLOCK;
	layout_children(root);
}

static void render_div(qui_div_t *d)
{
	if (!d || d->style->display == UI_DISPLAY_NONE)
		return;

	const qui_style_t *s = d->style;
	qui_div_t *c;

	/* background */
	if (s->bg_tex)
		qgl_tex_draw(s->bg_tex, d->x, d->y, d->w, d->h);
	else if (s->bg_color)
		qgl_fill(d->x, d->y, d->w, d->h, s->bg_color);

	/* border */
	if (s->border_size) {
		uint32_t b = s->border_size;
		uint32_t col = s->border_color;

		/* top */
		qgl_fill(d->x, d->y, d->w, b, col);
		/* bottom */
		qgl_fill(d->x, d->y + (int32_t)d->h - (int32_t)b, d->w, b, col);
		/* left */
		qgl_fill(d->x, d->y, b, d->h, col);
		/* right */
		qgl_fill(d->x + (int32_t)d->w - (int32_t)b, d->y, b, d->h, col);
	}

	/* text (with padding and clipping) */
	if (d->text && s->font_ref != QM_MISS) {
		int32_t tx = d->x + s->pad_left;
		int32_t ty = d->y + s->pad_top;
		int32_t tw = d->w - (s->pad_left + s->pad_right);
		int32_t th = d->h - (s->pad_top + s->pad_bottom);

		if (tw > 0 && th > 0) {
			// Enable scissor test for proper text clipping
			/* qgl_scissor(tx, ty, tw, th); */

			qgl_tint(s->text_color ? s->text_color : qgl_default_tint);

			qgl_font_draw(s->font_ref, d->text,
					tx, ty,
					tx + tw,  // explicit clip boundaries
					ty + th,
					s->font_scale);

			qgl_tint(qgl_default_tint);
			/* qgl_scissor_disable();  // disable scissor after text */
		}
	}

	for (c = d->first_child; c; c = c->next_sibling)
		render_div(c);
}

void qui_render(qui_div_t *root)
{
	render_div(root);
}

void qui_clear(qui_div_t *root)
{
	qui_div_t *c, *n;

	if (!root)
		return;

	c = root->first_child;
	while (c) {
		n = c->next_sibling;
		qui_clear(c);
		free(c);
		c = n;
	}

	root->first_child = NULL;
	qui_mark_dirty(root);
}

const char *qui_overflow(const qui_div_t *div)
{
	return div ? div->overflow : 0;
}
