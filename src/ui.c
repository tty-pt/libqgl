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
	s->position = UI_POS_RELATIVE;
	s->dir = UI_COLUMN;
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
	if (src->bg_tex != QM_MISS)
		dst->bg_tex = src->bg_tex;
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

	MERGE(grow);
	MERGE(shrink);
	MERGE(basis);
	MERGE(dir);
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
}

static void apply_style_recursive(qui_div_t *div,
				  qui_style_rule_t *ss,
				  const qui_style_t *inherited)
{
	const qui_style_t *class_style;
	qui_style_t merged = *inherited;
	qui_div_t *c;

	class_style = qui_stylesheet_lookup(ss, div->class_name);

	qui_style_merge(&merged, class_style);
	qui_style_merge(&merged, div->style);

	*div->style = merged;

	merged.position = UI_POS_RELATIVE; 
	merged.border_color = 0;
	merged.border_size = 1;
	merged.bg_color = 0;
	merged.left = merged.right
		= merged.top = merged.bottom = 0;
	merged.pad_top = merged.pad_bottom
		= merged.pad_left = merged.pad_right = 0;
	merged.font_scale = 1;

	for (c = div->first_child; c; c = c->next_sibling)
		apply_style_recursive(c, ss, &merged);
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

	/* measure children first */
	for (qui_div_t *ch = c->first_child;
			ch; ch = ch->next_sibling)
	{
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

	/* measure text if no children */
	if (!c->first_child && c->text
			&& c->style->font_ref != QM_MISS)
	{
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

	/* add padding */
	inner_w += c->style->pad_left + c->style->pad_right;
	inner_h += c->style->pad_top + c->style->pad_bottom;

	if (!c->w)
		c->w = inner_w;
	if (!c->h)
		c->h = inner_h;

	c->content_w = inner_w;
	c->content_h = inner_h;
}

static void layout_relative_child(
		qui_div_t *p, qui_div_t *c,
		int32_t *cur_x, int32_t *cur_y,
		uint32_t count,
		int32_t cur_w, int32_t cur_h)
{
	c->x = *cur_x;
	c->y = *cur_y;

	if (p->style->dir == UI_ROW) {
		c->h = cur_h;
		c->w = c->style->basis
			? c->style->basis
			: (cur_w / count);
		*cur_x += c->w;
	} else {
		c->w = cur_w;
		c->h = c->style->basis
			? c->style->basis
			: (cur_h / count);
		*cur_y += c->h;
	}
}

static void layout_absolute_child(qui_div_t *p, qui_div_t *c)
{
	/* horizontal positioning */
	if (c->style->left && c->style->right) {
		c->x = p->x + c->style->left;
		c->w = p->w - c->style->left - c->style->right;
	} else if (c->style->left) {
		c->x = p->x + c->style->left;
	} else if (c->style->right) {
		c->x = p->x + (int32_t)p->w - (int32_t)c->w
			- c->style->right;
	} else {
		c->x = p->x;
	}

	/* vertical positioning */
	if (c->style->top && c->style->bottom) {
		c->y = p->y + c->style->top;
		c->h = p->h - c->style->top - c->style->bottom;
	} else if (c->style->top) {
		c->y = p->y + c->style->top;
	} else if (c->style->bottom) {
		c->y = p->y + (int32_t)p->h - (int32_t)c->h
			- c->style->bottom;
	} else {
		c->y = p->y;
	}
}

static void measure_text_overflow(qui_div_t *c)
{
	if (!c->text || c->style->font_ref == QM_MISS)
		return;

	int32_t tx = c->x + c->style->pad_left;
	int32_t ty = c->y + c->style->pad_top;
	int32_t tw = c->w - (c->style->pad_left
			+ c->style->pad_right);
	int32_t th = c->h - (c->style->pad_top
			+ c->style->pad_bottom);

	if (tw <= 0 || th <= 0)
		return;

	uint32_t cw, ch;
	c->overflow = qgl_font_measure(&cw, &ch,
			c->style->font_ref, c->text,
			tx, ty,
			tx + (int32_t)tw,
			ty + (int32_t)th,
			c->style->font_scale);
}

static void layout_children(qui_div_t *p)
{
	if (!p->first_child)
		return;

	/* measure recursively first */
	for (qui_div_t *c = p->first_child; c; c = c->next_sibling)
		if (c->style->display != UI_DISPLAY_NONE)
			measure_content_size(c);

	qui_div_t *c;
	uint32_t count = 0;
	int32_t cur_x, cur_y;
	uint32_t cur_w, cur_h;

	for (c = p->first_child; c; c = c->next_sibling)
		if (c->style->display != UI_DISPLAY_NONE
				&& c->style->position == UI_POS_RELATIVE)
			count++;

	cur_x = p->x + p->style->pad_left;
	cur_y = p->y + p->style->pad_top;
	cur_w = p->w - p->style->pad_left - p->style->pad_right;
	cur_h = p->h - p->style->pad_top - p->style->pad_bottom;

	for (c = p->first_child; c; c = c->next_sibling) {
		if (c->style->display == UI_DISPLAY_NONE)
			continue;

		if (c->style->position == UI_POS_RELATIVE)
			layout_relative_child(p, c,
					&cur_x, &cur_y,
					count, cur_w, cur_h);
		else
			layout_absolute_child(p, c);

		measure_text_overflow(c);
		layout_children(c);
	}

	p->content_w = cur_x - p->x;
	p->content_h = cur_y - p->y;
}

void qui_layout(qui_div_t *root, int32_t x, int32_t y,
		uint32_t w, uint32_t h)
{
	root->x = x;
	root->y = y;
	root->w = w;
	root->h = h;
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

	/* text (with padding) */
	if (d->text && s->font_ref != QM_MISS) {
		int32_t tx = d->x + s->pad_left;
		int32_t ty = d->y + s->pad_top;
		int32_t tw = d->w - (s->pad_left + s->pad_right);
		int32_t th = d->h - (s->pad_top + s->pad_bottom);

		if (tw > 0 && th > 0) {
			qgl_tint(s->text_color
					? s->text_color
					: qgl_default_tint);

			qgl_font_draw(s->font_ref, d->text,
				      tx, ty,
				      tx + (int32_t)tw,
				      ty + (int32_t)th,
				      s->font_scale);

			qgl_tint(qgl_default_tint);
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
}

const char *qui_overflow(const qui_div_t *div)
{
	return div ? div->overflow : 0;
}
