#include "./ui.h"
#include "./ui-cache.h"
#include "../include/ttypt/qgl-font.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <ttypt/qmap.h>

#define MAX_CHILDREN 256

typedef struct qui_container {
	qui_div_t *node;
	int32_t inner_x, inner_y;
	uint32_t inner_w, inner_h;
	float inner_main, inner_cross;
	float line_x, line_y;
} qui_container_t;

static uint32_t g_screen_w, g_screen_h;
static qui_style_t style_default;

static void qui_style_default(qui_style_t *s)
{
	s->font_size = 1;
	s->display = QUI_DISPLAY_BLOCK;

	s->align_items = QUI_ALIGN_STRETCH;
	s->justify_content = QUI_JUSTIFY_FLEX_START;
	s->align_self = QUI_ALIGN_AUTO;

	s->position = QUI_POSITION_RELATIVE;
	s->left = s->right = QUI_AUTO;
	s->top  = s->bottom = QUI_AUTO;

	s->padding_top = s->padding_bottom = 0;
	s->padding_left = s->padding_right = 0;
	s->border_color = 0;
	s->border_width = 0;
	s->background_color = 0;
	s->background_image_ref = 0;
	s->flex_grow = 0;
	s->flex_shrink = 1;
	s->flex_basis = QUI_AUTO;
	s->flex_direction = QUI_COLUMN;

	s->border_radius_top_left =
		s->border_radius_top_right =
		s->border_radius_bottom_right =
		s->border_radius_bottom_left = 0;

	s->box_shadow_color = 0;
	s->box_shadow_blur = 0.0f;
	s->box_shadow_offset_x = 0.0f;
	s->box_shadow_offset_y = 0.0f;

	s->text_align = QUI_TEXT_ALIGN_LEFT;
}

void qui_style_reset(qui_style_t *s)
{
	memset(s, 0, sizeof(*s));
	qui_style_default(s);
}

static void qui_style_merge(qui_style_t *dst, const qui_style_t *src)
{
	if (!src)
		return;

#define MERGE(f) do { \
	if (src->f != style_default.f) \
	dst->f = src->f; \
} while (0)

		MERGE(background_color);
		MERGE(border_color);
		MERGE(border_width);
		MERGE(background_image_ref);
		MERGE(font_family_ref);
		MERGE(font_size);
		MERGE(color);

		MERGE(align_items);
		MERGE(justify_content);
		MERGE(align_self);

		MERGE(padding_left);
		MERGE(padding_right);
		MERGE(padding_top);
		MERGE(padding_bottom);

		MERGE(position);
		MERGE(left);
		MERGE(right);
		MERGE(top);
		MERGE(bottom);

		MERGE(display);
		MERGE(flex_grow);
		MERGE(flex_basis);
		MERGE(flex_shrink);
		MERGE(flex_direction);

		MERGE(border_radius_top_left);
		MERGE(border_radius_top_right);
		MERGE(border_radius_bottom_right);
		MERGE(border_radius_bottom_left);

		MERGE(box_shadow_color);
		MERGE(box_shadow_blur);
		MERGE(box_shadow_offset_x);
		MERGE(box_shadow_offset_y);

		MERGE(text_align);
		MERGE(white_space);
		MERGE(word_break);
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
	qui_style_reset(&style_default);
}

static void qui_invalidate_up(qui_div_t *d)
{
    for (qui_div_t *p = d; p; p = p->parent)
        qgl_cache_invalidate(p);
}

static void qui_mark_dirty(qui_div_t *d)
{
	if (!d)
		return;

	d->w = d->h = 0;
	qui_invalidate_up(d);

	for (qui_div_t *c = d->first_child;
			c;
			c = c->next_sibling)

		qui_mark_dirty(c);
}

qui_div_t *qui_new(qui_div_t *parent, qui_style_t *style)
{
	qui_div_t *d = calloc(1, sizeof(*d));

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

	if (!parent)
		return d;

	d->parent = parent;

	if (!parent->first_child) {
		parent->first_child = d;
		parent->last_child = d;
	} else {
		parent->last_child->next_sibling = d;
		parent->last_child = d;
	}

	qui_mark_dirty(parent);
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

	class_style = qui_stylesheet_lookup(ss,
			div->class_name);

	qui_style_merge(&merged, class_style);

	if (!div->style_calloc)
		qui_style_merge(&merged, div->style);

	*div->style = merged;

	/* inheritance */
	qui_style_t child_inh = *div->style;
	qui_style_default(&child_inh);

	for (c = div->first_child; c; c = c->next_sibling)
		apply_style_recursive(c, ss, &child_inh);
}

void qui_apply_styles(qui_div_t *root, qui_style_rule_t *ss)
{
	qui_style_t base;

	qui_style_reset(&base);
	apply_style_recursive(root, ss, &base);
}

static void measure_content_size(
		qui_div_t *c,
		uint32_t avail_w,
		uint32_t avail_h)
{
	uint32_t inner_w = 0, inner_h = 0;
	const int flex_zero = (c->style->flex_basis == 0 && c->style->flex_grow > 0.0f);

	/* 1. measure children and text */
	for (qui_div_t *ch = c->first_child; ch; ch = ch->next_sibling) {
		if (ch->style->display == QUI_DISPLAY_NONE)
			continue;

		measure_content_size(ch, avail_w, avail_h);

		if (c->style->flex_direction == QUI_ROW) {
			inner_w += ch->content_w;
			if (ch->content_h > inner_h)
				inner_h = ch->content_h;
		} else {
			inner_h += ch->content_h;
			if (ch->content_w > inner_w)
				inner_w = ch->content_w;
		}
	}

	/* 2. measure text */
	if (!c->first_child && c->text && c->style->font_family_ref != QM_MISS)
	{
		uint32_t tw = 0, th = 0;
		qgl_font_measure(&tw, &th,
				c->style->font_family_ref, c->text,
				0, 0, avail_w, avail_h,
				c->style->font_size,
				c->style->white_space,
				c->style->word_break);
		if (tw > inner_w) inner_w = tw;
		if (th > inner_h) inner_h = th;
	}

	/* 3. minimal fallback for basis < 0 without children or text */
	if (c->style->flex_basis < 0 && !c->first_child && !c->text) {
		if (c->parent && c->parent->style->flex_direction == QUI_ROW)
			inner_w = inner_w ? inner_w : 1;
		else
			inner_h = inner_h ? inner_h : 1;
	}

	/* 4. flex_zero logic applied after measurement.
	 * Preserves cross axis measurement, but replaces
	 * main axix.
	 */
	if (flex_zero) {
		if (c->parent && c->parent->style->flex_direction == QUI_ROW)
			inner_w = 1;
		else
			inner_h = 1; /* Overwrite main axis */
	}

	/* 5. padding */
	inner_w += c->style->padding_left + c->style->padding_right;
	inner_h += c->style->padding_top + c->style->padding_bottom;

	/* Add border size, as it's part of the border-box */
	if (c->style->border_width > 0) {
		inner_w += c->style->border_width * 2;
		inner_h += c->style->border_width * 2;
	}

	/* 6. Define w/h if base != 0 */
	if (!flex_zero) {
		if (!c->w)
			c->w = inner_w;
		if (!c->h)
			c->h = inner_h;
	}

	/* 7. Always store the measurement result */
	c->content_w = inner_w;
	c->content_h = inner_h;
}

static void measure_text_overflow(qui_div_t *c)
{
	c->overflow = NULL;

	if (!c->text || c->style->font_family_ref == QM_MISS)
		return;

	int32_t border_w = c->style->border_width * 2;
	int32_t border_h = c->style->border_width * 2;

	int32_t tw = (int32_t) c->w
		- c->style->padding_left
		- c->style->padding_right
		- border_w;

	int32_t th = (int32_t) c->h
		- c->style->padding_top
		- c->style->padding_bottom
		- border_h;

	if (tw <= 0 || th <= 0) {
		c->overflow = c->text; /* nothing fits */
		return;
	}

	uint32_t mw = 0, mh = 0;

	c->overflow = qgl_font_measure(&mw, &mh,
			c->style->font_family_ref, c->text,
			0, 0, (uint32_t)tw, (uint32_t)th,
			c->style->font_size,
			c->style->white_space,
			c->style->word_break);
}

/*
 * clampf — constrain a float to [lo, hi]
 *
 * Returns:
 *   lo if v < lo
 *   hi if v > hi
 *   v otherwise
 */
static inline float clampf(float v, float lo, float hi)
{
	if (v < lo)
		return lo;

	if (v > hi)
		return hi;

	return v;
}

/*
 * Step 1: Compute base sizes and hypothetical main sizes
 *
 * For each flex item:
 *  - flex-basis > 0   → use that value.
 *  - flex-basis == 0  → skip content measurement (base 0)
 *  - flex-basis < 0   → "auto": use width/height if
 *  	specified, otherwise measure intrinsic content.
 *
 * The result is stored in child->w or child->h
 * depending on container direction.
 */

static void compute_base_sizes(qui_container_t *c, qui_div_t **items, int count)
{
	for (int i = 0; i < count; i++) {
		qui_div_t *child = items[i];

		if (!child->style || child->style->display == QUI_DISPLAY_NONE)
			continue;

		qui_style_t *s = child->style;
		float base_main = 0.f;

		/* use measured content if it already exists */
		float content_main = c->node->style->flex_direction == QUI_ROW
			? (float)child->content_w
			: (float)child->content_h;

		if (s->flex_basis > 0)
			base_main = (float)s->flex_basis;
		else if (s->flex_basis == 0)
			base_main = 0.f;
		else /* basis < 0 → auto */
			base_main = (content_main > 0)
				? content_main
				: 1.0f;

		/* don't overwrite if there are already
		 * valid w/h from measure_content_size
		 */
		if (c->node->style->flex_direction == QUI_ROW) {
			if (child->w == 0)
				child->w = (uint32_t)
					clampf(base_main, 1, 1e9f);
		} else {
			if (child->h == 0)
				child->h = (uint32_t)
					clampf(base_main, 1, 1e9f);
		}
	}
}


/*
 * Grow phase: distribute positive free space by flex-grow
 */
static void distribute_flex_grow(qui_container_t *c,
		qui_div_t **items, int count, float free_space)
{
	float total_grow = 0.f;

	for (int i = 0; i < count; i++) {
		qui_div_t *ch = items[i];

		if (!ch->style || ch->style->display == QUI_DISPLAY_NONE)
			continue;

		total_grow += ch->style->flex_grow;
	}

	if (total_grow <= 0.f)
		return;

	for (int i = 0; i < count; i++) {
		qui_div_t *ch = items[i];

		if (!ch->style || ch->style->display == QUI_DISPLAY_NONE)
			continue;

		float add = (ch->style->flex_grow / total_grow) * free_space;

		if (c->node->style->flex_direction == QUI_ROW)
			ch->w = (uint32_t) clampf((float)ch->w + add, 0, 1e9);
		else
			ch->h = (uint32_t) clampf((float)ch->h + add, 0, 1e9);
	}
}

/*
 * Shrink phase: distribute negative free space by flex-shrink × base
 */
static void distribute_flex_shrink(qui_container_t *c,
		qui_div_t **items, int count, float free_space)
{
	float total_scaled = 0.f;

	for (int i = 0; i < count; i++) {
		qui_div_t *ch = items[i];

		if (!ch->style || ch->style->display == QUI_DISPLAY_NONE)
			continue;

		float basis = (c->node->style->flex_direction == QUI_ROW)
			? (float)ch->w : (float)ch->h;

		total_scaled += ch->style->flex_shrink * basis;
	}

	if (total_scaled <= 0.f)
		return;

	for (int i = 0; i < count; i++) {
		qui_div_t *ch = items[i];
		if (!ch->style || ch->style->display == QUI_DISPLAY_NONE)
			continue;

		float basis = (c->node->style->flex_direction == QUI_ROW)
			? (float)ch->w : (float)ch->h;
		float scaled = ch->style->flex_shrink * basis;

		/* free_space < 0 */
		float sub = (scaled / total_scaled) * free_space;
		float new_size = basis + sub;

		if (new_size < 0.f)
			new_size = 0.f;

		if (c->node->style->flex_direction == QUI_ROW)
			ch->w = (uint32_t)new_size;
		else
			ch->h = (uint32_t)new_size;
	}
}

/*
 * Step 2: Distribute free space (grow / flex_shrink)
 *
 * For each flex line:
 *  - Compute free_space = inner_main - Σ(base_main)
 *  - If free_space > 0  → distribute by flex-grow
 *  - If free_space < 0  → distribute by flex-shrink × base_main
 *  - Clamp results >= 0
 */
static void distribute_free_space(qui_container_t *c,
		qui_div_t **items, int count)
{
	float total_basis = 0.f;

	for (int i = 0; i < count; i++) {
		qui_div_t *child = items[i];

		if (!child->style || child->style->display == QUI_DISPLAY_NONE)
			continue;

		if (c->node->style->flex_direction == QUI_ROW)
			total_basis += (float)child->w;
		else
			total_basis += (float)child->h;
	}

	float free_space = c->inner_main - total_basis;

	if (free_space >= 0.f)
		distribute_flex_grow(c, items, count, free_space);
	else if (free_space < 0.f)
		distribute_flex_shrink(c, items, count, free_space);
}

/*
 * Step 3: Main-axis positioning (justify-content)
 *
 * After all items on this flex line have their final main sizes:
 *   - Sum total used space
 *   - Compute remaining free space
 *   - Distribute according to justify-content
 *
 * Works for both QUI_ROW and QUI_COLUMN directions.
 */

static void position_main_axis(qui_container_t *c,
		qui_div_t **items, int count,
		float line_main_origin_x,
		float line_main_origin_y)
{
	if (count == 0)
		return;

	/* 1) total used main size */
	float total_used = 0.f;
	for (int i = 0; i < count; i++) {
		qui_div_t *ch = items[i];
		if (!ch->style || ch->style->display == QUI_DISPLAY_NONE)
			continue;
		total_used += (c->node->style->flex_direction == QUI_ROW)
			? (float)ch->w : (float)ch->h;
	}

	float free_space = c->inner_main - total_used;
	if (free_space < 0.f) free_space = 0.f; /* overflow handled earlier */

	/* 2) determine spacing and start offset */
	float start = 0.f, gap = 0.f;

	switch (c->node->style->justify_content) {
		case QUI_JUSTIFY_FLEX_START:
			start = 0.f;
			break;

		case QUI_JUSTIFY_FLEX_END:
			start = free_space;
			break;

		case QUI_JUSTIFY_CENTER:
			start = free_space * 0.5f;
			break;

		case QUI_JUSTIFY_SPACE_BETWEEN:
			if (count > 1)
				gap = free_space / (float)(count - 1);
			else
				start = free_space * 0.5f;
			break;

		case QUI_JUSTIFY_SPACE_AROUND:
			gap = free_space / (float)count;
			start = gap * 0.5f;
			break;
	}

	/* 3) assign positions along main axis */
	float cursor = start;
	for (int i = 0; i < count; i++) {
		qui_div_t *ch = items[i];
		if (!ch->style || ch->style->display == QUI_DISPLAY_NONE)
			continue;

		if (c->node->style->flex_direction == QUI_ROW) {
			ch->x = (int32_t)(line_main_origin_x + cursor);
			ch->y = (int32_t)line_main_origin_y;
			cursor += (float)ch->w + gap;
		} else {
			ch->x = (int32_t)line_main_origin_x;
			ch->y = (int32_t)(line_main_origin_y + cursor);
			cursor += (float)ch->h + gap;
		}
	}
}

/*
 * Step 4: Cross-axis positioning (align-items / align-self)
 *
 * For each item on the line:
 *   - Determine target cross size
 *   - Apply align-self (or container align-items)
 *   - Place item within the line’s cross area
 *
 * The line itself has already been assigned its cross origin
 * and cross size by the multi-line driver.
 */
static void position_cross_axis(qui_container_t *c,
                                qui_div_t **items,
				int count)
{
    for (int i = 0; i < count; i++) {
        qui_div_t *child = items[i];

        if (!child->style || child->style->display == QUI_DISPLAY_NONE)
            continue;

        qui_style_t *s = child->style;

        /* 1) choose effective alignment */
        qui_align_mode_t align
		= c->node->style->align_items;

        if (s->align_self != QUI_ALIGN_AUTO)
            align = s->align_self;

        /* 2) compute available space on the cross axis */
        float cross_inner = (c->node->style->flex_direction == QUI_ROW)
            ? c->inner_h
            : c->inner_w;

        /* 3) stretch logic */
        if (align == QUI_ALIGN_STRETCH) {
            if (c->node->style->flex_direction == QUI_ROW)
                child->h = (uint32_t)clampf(cross_inner, 0.f, 1e9f);
            else
                child->w = (uint32_t)clampf(cross_inner, 0.f, 1e9f);
        }

	/* 4) determine item cross size */
	float item_cross = (c->node->style->flex_direction == QUI_ROW)
		? (float)child->h
		: (float)child->w;

	float offset = 0.f;

	switch (align) {
		case QUI_ALIGN_FLEX_END:
			offset = cross_inner - item_cross;
			break;
		case QUI_ALIGN_CENTER:
			offset = (cross_inner - item_cross) * 0.5f;
			break;
		default:
			offset = 0.f;
			break;
	}


        /* 7) apply final position — fix: don't add padding twice */
        if (c->node->style->flex_direction == QUI_ROW)
            child->y = (int32_t)(c->inner_y + offset);
        else
            child->x = (int32_t)(c->inner_x + offset);
    }
}

static void qui_container_inner_box(qui_div_t *container,
		float *x, float *y,
		float *w, float *h)
{
	const qui_style_t *s = container->style;
	*x = container->x + s->padding_left;
	*y = container->y + s->padding_top;
	*w = container->w - (s->padding_left + s->padding_right);
	*h = container->h - (s->padding_top + s->padding_bottom);

	if (*w < 0)
		*w = 0;

	if (*h < 0)
		*h = 0;
}

static int qui_collect_children(
		int *flex_n_r, int *abs_n_r,
		qui_div_t *container,
		qui_div_t **flex_out,
		qui_div_t **abs_out)
{
	int flex_n = 0, abs_n = 0;

	for (qui_div_t *c = container->first_child;
			c;
			c = c->next_sibling)
	{
		if (!c->style || c->style->display == QUI_DISPLAY_NONE)
			continue;

		if (c->style->position == QUI_POSITION_ABSOLUTE)
			abs_out[abs_n++] = c;
		else
			flex_out[flex_n++] = c;
	}

	*abs_n_r = abs_n;
	*flex_n_r = flex_n;
	return flex_n;
}

static void qui_flex_layout_line(qui_container_t *ctx,
		qui_div_t **items, int count,
		float *inner_x, float *inner_y)
{
	compute_base_sizes(ctx, items, count);
	distribute_free_space(ctx, items, count);

	for (int i = 0; i < count; i++) {
		qui_div_t *ch = items[i];
		measure_content_size(ch, ch->w, ch->h);

		if (ch->style->flex_basis >= 0)
			continue;

		if (ctx->node->style->flex_direction == QUI_ROW)
			ch->h = ch->content_h;
		else
			ch->w = ch->content_w;
	}

	float line_cross = 0.f;
	for (int i = 0; i < count; i++) {
		float cross = (ctx->node->style->flex_direction == QUI_ROW)
			? (float)items[i]->h
			: (float)items[i]->w;

		if (cross > line_cross)
			line_cross = cross;
	}

	position_main_axis(ctx, items, count, *inner_x, *inner_y);
	position_cross_axis(ctx, items, count);

	if (ctx->node->style->flex_direction == QUI_ROW)
		*inner_y += line_cross;
	else
		*inner_x += line_cross;
}

/* Position absolute position children relative to container */
static void qui_layout_absolute_children(
		qui_div_t *container,
		qui_div_t **abs_children)
{
	/* optional array  - recompute from the tree */
	(void) abs_children;

	if (!container || !container->style)
		return;

	for (qui_div_t *a = container->first_child;
			a;
			a = a->next_sibling)
	{
		if (!a->style || a->style->display == QUI_DISPLAY_NONE)
			continue;

		if (a->style->position != QUI_POSITION_ABSOLUTE)
			continue;

		qui_style_t *as = a->style;

		/* X axis */
		if (as->left != QUI_AUTO && as->right != QUI_AUTO) {
			a->x = container->x + as->left;

			int32_t right_edge = container->x
				+ (int32_t)container->w - as->right;

			a->w = (uint32_t)
				clampf((float)(right_edge - a->x),
						0.f, 1e9f);
		} else if (as->left != QUI_AUTO)
			a->x = container->x + as->left;
		else if (as->right != QUI_AUTO)
			a->x = container->x + (int32_t)container->w - as->right - (int32_t)a->w;
		else
			a->x = container->x;

		/* Y axis */
		if (as->top != QUI_AUTO && as->bottom != QUI_AUTO) {
			a->y = container->y + as->top;

			int32_t bottom_edge = container->y
				+ (int32_t)container->h - as->bottom;

			a->h = (uint32_t)
				clampf((float)(bottom_edge - a->y),
						0.f, 1e9f);
		} else if (as->top != QUI_AUTO)
			a->y = container->y + as->top;
		else if (as->bottom != QUI_AUTO)
			a->y = container->y + (int32_t)container->h
				- as->bottom - (int32_t)a->h;
		else
			a->y = container->y;
	}
}

static void qui_layout_flex(qui_div_t *container);

/* Measure overflow and do recursive child layout */
static void qui_detect_overflow_and_recurse(qui_div_t *container)
{
	if (!container)
		return;

	if (!container->style || container->style->display == QUI_DISPLAY_NONE)
		return;

	measure_text_overflow(container);

	for (qui_div_t *c = container->first_child;
			c;
			c = c->next_sibling)

		qui_layout_flex(c);
}

static void qui_layout_flex(qui_div_t *container)
{
	if (!container || !container->style
			|| container->style->display == QUI_DISPLAY_NONE)
		return;

	if (container->w == 0 || container->h == 0)
		return;

	float inner_x, inner_y, inner_w, inner_h;
	qui_container_inner_box(container,
			&inner_x, &inner_y,
			&inner_w, &inner_h);

	for (qui_div_t *c = container->first_child; c; c = c->next_sibling)
		measure_content_size(c, inner_w, inner_h);

	qui_div_t *abs_children[MAX_CHILDREN];
	qui_div_t *flex_children[MAX_CHILDREN];
	int flex_n, abs_n;
       	qui_collect_children(&flex_n, &abs_n, container,
			flex_children, abs_children);

	qui_container_t ctx = {
		.node = container,
		.inner_x = inner_x,
		.inner_y = inner_y,
		.inner_w = inner_w,
		.inner_h = inner_h,
		.inner_main  = container->style->flex_direction == QUI_ROW
			? inner_w : inner_h,
		.inner_cross = container->style->flex_direction == QUI_ROW
			? inner_h : inner_w,
	};

	float cursor_x = inner_x, cursor_y = inner_y;
	qui_flex_layout_line(&ctx, flex_children,
			flex_n, &cursor_x, &cursor_y);

	qui_layout_absolute_children(container, abs_children);
	qui_detect_overflow_and_recurse(container);
}

void qui_layout(qui_div_t *root, int32_t x, int32_t y,
		uint32_t w, uint32_t h)
{
	if (!root || !root->style)
		return;

	if (w == 0)
		w = g_screen_w;
	if (h == 0)
		h = g_screen_h;

	root->x = x;
	root->y = y;

	root->w = w;
	root->h = h;

	/* Force visible root block */
	root->style->display = QUI_DISPLAY_BLOCK;

	/* Use the new flex-based layout entry */
	qui_layout_flex(root);
}

/*
 * render_div_raw — low-level draw: paints the element and children
 * without ever building or drawing from cache.
 */
void render_div_raw(qui_div_t *d)
{
	if (!d || d->style->display == QUI_DISPLAY_NONE)
		return;

	const qui_style_t *s = d->style;

	/* box-shadow */
	if (s->box_shadow_color && s->box_shadow_blur > 0.0f) {
		qgl_box_shadow(
				s->box_shadow_color,
				d->x,
				d->y,
				d->w,
				d->h,
				(float)s->border_radius_top_left,
				(float)s->border_radius_top_right,
				(float)s->border_radius_bottom_right,
				(float)s->border_radius_bottom_left,
				s->box_shadow_blur,
				s->box_shadow_offset_x,
				s->box_shadow_offset_y
			      );
	}

	/* background */
	if (s->background_image_ref) {
		qgl_tex_draw(s->background_image_ref, d->x, d->y, d->w, d->h);
	} else if (s->background_color || s->border_width) {
		int has_radius =
			s->border_radius_top_left |
			s->border_radius_top_right |
			s->border_radius_bottom_right |
			s->border_radius_bottom_left;

		if (has_radius) {
			qgl_border_radius(
				s->background_color,
				s->border_color,
				d->x, d->y, d->w, d->h,
				(float)s->border_radius_top_left,
				(float)s->border_radius_top_right,
				(float)s->border_radius_bottom_right,
				(float)s->border_radius_bottom_left,
				s->border_width
			);
		} else {
			if (s->background_color)
				qgl_fill(d->x, d->y, d->w, d->h, s->background_color);

			if (s->border_width) {
				uint32_t b = s->border_width, col = s->border_color;
				qgl_fill(d->x, d->y, d->w, b, col);
				qgl_fill(d->x, d->y + (int32_t)d->h - (int32_t)b, d->w, b, col);
				qgl_fill(d->x, d->y, b, d->h, col);
				qgl_fill(d->x + (int32_t)d->w - (int32_t)b, d->y, b, d->h, col);
			}
		}
	}

	/* text */
		if (d->text && s->font_family_ref != QM_MISS) {
		int32_t border_w = s->border_width * 2;
		int32_t border_h = s->border_width * 2;

		int32_t tx = d->x + s->padding_left + s->border_width;
		int32_t ty = d->y + s->padding_top + s->border_width;
		int32_t tw = d->w - (s->padding_left + s->padding_right) - border_w;
		int32_t th = d->h - (s->padding_top + s->padding_bottom) - border_h;

		if (tw > 0 && th > 0) {
			uint32_t mw = 0, mh = 0;
			qgl_font_measure(&mw, &mh,
				s->font_family_ref, d->text,
				0, 0, (uint32_t)tw, (uint32_t)th,
				s->font_size,
				s->white_space,
				s->word_break);

			int32_t align_x = tx;

			switch (s->text_align) {
			case QUI_TEXT_ALIGN_CENTER:
				align_x = tx + ((int32_t)tw - (int32_t)mw) / 2;
				break;
			case QUI_TEXT_ALIGN_RIGHT:
				align_x = tx + ((int32_t)tw - (int32_t)mw);
				break;
			default:
				break;
			}

			qgl_tint(s->color ? s->color : qgl_default_tint);
			qgl_font_draw(s->font_family_ref,
					d->text,
					align_x, ty,
					tx + tw, ty + th,
					s->font_size,
					s->white_space,
					s->word_break);
			qgl_tint(qgl_default_tint);
		}
	}

	for (qui_div_t *c = d->first_child; c; c = c->next_sibling)
		render_div_raw(c);
}

/*
 * render_div — high-level draw using caching.
 * If cache valid, draws cached texture; otherwise builds it.
 */
static void render_div(qui_div_t *d)
{
	if (!d || d->style->display == QUI_DISPLAY_NONE)
		return;

	if (!qgl_cache_valid(d))
		qgl_cache_build(d);

	qgl_cache_draw(d, d->x, d->y);
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

		if (c->style_calloc && c->style)
			free(c->style);

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
