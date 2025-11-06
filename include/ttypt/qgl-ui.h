#ifndef QGL_UI_H
#define QGL_UI_H

/**
 * @file qgl-ui.h
 * @brief Public header for the QGL UI system.
 *
 * Declares the API for QGL’s immediate-mode
 * layout and rendering system. Provides a small,
 * CSS-like model for composing 2D interface
 * elements (divs) with flexible layout and style.
 */

#include "qgl.h"
#include <stdint.h>

#define QUI_STYLE(div, field, val) \
	do { \
		qui_style_t dummy; \
		dummy.field = val; \
		qui_style_set(div, offsetof(qui_style_t, field), &dummy.field, sizeof(dummy.field)); \
	} while (0)

/** @defgroup qui_types QGL UI types
 * @brief Fundamental type definitions.
 * @{
 */

/**
 * @brief Automatic layout value.
 *
 * Used for style properties like `top`, `left`, `flex_basis`,
 * etc., to indicate that the layout engine should
 * determine the value automatically.
 */
static const int32_t QUI_AUTO = -1;

/**
 * @brief Layout direction for container elements (CSS: flex-direction).
 */
typedef enum {
	QUI_COLUMN,
	QUI_ROW,
} qui_flex_direction_t;

/**
 * @brief Positioning mode for elements (CSS: position).
 */
typedef enum {
	QUI_POSITION_RELATIVE,
	QUI_POSITION_ABSOLUTE,
} qui_position_mode_t;

/**
 * @brief Display mode for visibility control (CSS: display).
 */
typedef enum {
	QUI_DISPLAY_BLOCK,
	QUI_DISPLAY_INLINE,
	QUI_DISPLAY_FLEX,
	QUI_DISPLAY_NONE,
} qui_display_mode_t;


/**
 * @brief Alignment along cross axis (CSS: align-items, align-self).
 */
typedef enum {
	QUI_ALIGN_STRETCH = 0,
	QUI_ALIGN_FLEX_START,
	QUI_ALIGN_CENTER,
	QUI_ALIGN_FLEX_END,
	QUI_ALIGN_AUTO = -1,
} qui_align_mode_t;

/**
 * @brief Alignment of items along the main axis (CSS: justify-content).
 */
typedef enum {
	QUI_JUSTIFY_FLEX_START = 0,
	QUI_JUSTIFY_CENTER,
	QUI_JUSTIFY_FLEX_END,
	QUI_JUSTIFY_SPACE_BETWEEN,
	QUI_JUSTIFY_SPACE_AROUND
} qui_justify_content_mode_t;

typedef enum {
	QUI_TEXT_ALIGN_LEFT = 0,
	QUI_TEXT_ALIGN_CENTER,
	QUI_TEXT_ALIGN_RIGHT
} qui_text_align_t;

typedef enum {
	QUI_WS_NORMAL = 0,
	QUI_WS_NOWRAP,
	QUI_WS_PRE,
	QUI_WS_PRE_WRAP,
	QUI_WS_PRE_LINE
} qui_white_space_t;

typedef enum {
	QUI_WB_NORMAL = 0,
	QUI_WB_BREAK_ALL,
	QUI_WB_KEEP_ALL,
	QUI_WB_BREAK_WORD
} qui_word_break_t;

/**
 * @brief Visual style definition for a UI element.
 *
 * Represents colors, borders, fonts and layout
 * metrics. Styles may be defined globally in a
 * stylesheet and applied by class name.
 */
typedef struct {
	uint32_t background_color;      /**< CSS: background-color (BGRA). */
	uint32_t border_color;          /**< CSS: border-color (BGRA). */
	uint32_t border_width;          /**< CSS: border-width in pixels. */
	uint32_t background_image_ref;  /**< Texture reference (QM_MISS = none). */
	uint32_t font_family_ref;       /**< Font reference (QM_MISS = none). */
	uint32_t font_size;             /**< CSS: font-size (as a scale factor). */
	uint32_t color;                 /**< CSS: color (text color, BGRA). */

	uint32_t padding_left;          /**< CSS: padding-left in pixels. */
	uint32_t padding_right;         /**< CSS: padding-right in pixels. */
	uint32_t padding_top;           /**< CSS: padding-top in pixels. */
	uint32_t padding_bottom;        /**< CSS: padding-bottom in pixels. */

	/* positioning */
	qui_position_mode_t position;   /**< CSS: position (relative or absolute). */
	int32_t  top;                   /**< CSS: top offset or QUI_AUTO. */
	int32_t  left;                  /**< CSS: left offset or QUI_AUTO. */
	int32_t  right;                 /**< CSS: right offset or QUI_AUTO. */
	int32_t  bottom;                /**< CSS: bottom offset or QUI_AUTO. */

	qui_display_mode_t  display;    /**< CSS: display (visibility and layout mode). */

	/* flexbox layout */
	qui_flex_direction_t flex_direction; /**< CSS: flex-direction (QUI_ROW or QUI_COLUMN). */
	float flex_grow;                     /**< CSS: flex-grow factor. */
	float flex_shrink;                   /**< CSS: flex-shrink factor. */
	int32_t flex_basis;                  /**< CSS: flex-basis (pixels or QUI_AUTO). */

	/* flex alignment properties */
	qui_align_mode_t   align_items;     /**< CSS: align-items (Cross-axis alignment of children). */
	qui_align_mode_t   align_self;      /**< CSS: align-self (Cross-axis alignment of self). */
	qui_justify_content_mode_t justify_content; /**< CSS: justify-content (Main-axis alignment of children). */

	uint32_t border_radius_top_left;     /**< CSS: border-top-left-radius in pixels. */
	uint32_t border_radius_top_right;    /**< CSS: border-top-right-radius in pixels. */
	uint32_t border_radius_bottom_right; /**< CSS: border-bottom-right-radius in pixels. */
	uint32_t border_radius_bottom_left;  /**< CSS: border-bottom-left-radius in pixels. */

	uint32_t box_shadow_color;     /**< CSS: box-shadow color (BGRA). */
	float    box_shadow_blur;      /**< CSS: box-shadow blur radius in pixels. */
	float    box_shadow_offset_x;  /**< CSS: box-shadow horizontal offset. */
	float    box_shadow_offset_y;  /**< CSS: box-shadow vertical offset. */

	qui_text_align_t text_align;
	qui_white_space_t white_space;
	qui_word_break_t  word_break;
} qui_style_t;

/**
 * @brief A single stylesheet rule.
 */
typedef struct qui_style_rule {
	const char *class_name;     /**< The class name this rule targets. */
	qui_style_t style;          /**< The style properties for this rule. */
	struct qui_style_rule *next;/**< Pointer to the next rule in the list. */
} qui_style_rule_t;

/**
 * @brief Opaque UI element.
 *
 * Each div represents a node in the layout tree.
 * You can set its class, text, flex behavior and
 * nesting dynamically.
 */
typedef struct qui_div qui_div_t;

/** @} */

/** @defgroup qui_core QGL UI initialization and stylesheet
 * @brief Core setup and stylesheet management.
 * @{
 */

/**
 * @brief Initialize the UI subsystem.
 *
 * Sets up internal dimensions and prepares the
 * layout engine for rendering.
 */
void qui_init(uint32_t screen_w, uint32_t screen_h);

/**
 * @brief Initialize (or reset) a stylesheet.
 *
 * Sets *ss to NULL, clearing any existing rules.
 */
void qui_stylesheet_init(qui_style_rule_t **ss);

/**
 * @brief Add a new class rule to a stylesheet.
 *
 * @param[in,out] ss         Stylesheet handle.
 * @param[in]     class_name CSS-like class identifier.
 * @param[in]     style      Style attributes to store.
 */
void qui_stylesheet_add(qui_style_rule_t **ss,
		const char *class_name,
		const qui_style_t *style);

/** @} */

/** @defgroup qui_tree QGL UI element tree
 * @brief Element creation and configuration.
 * @{
 */

/**
 * @brief Create a new UI element.
 *
 * @param[in] parent Parent div, or NULL for root.
 * @param[in] style  Optional pointer to a style struct
 * for inline styles. Can be NULL.
 * @return           Pointer to new element.
 */
qui_div_t *qui_new(qui_div_t *parent, qui_style_t *style);

/**
 * @brief Assign a CSS-like class to a div.
 */
void qui_class(qui_div_t *div, const char *class_name);

/**
 * @brief Assign text content to a div.
 */
void qui_text(qui_div_t *div, const char *text);

/**
 * @brief Query overflow continuation text.
 *
 * After rendering, returns the text pointer at
 * which drawing stopped (if overflow occurred).
 *
 * @param[in] div Target element.
 * @return        Pointer to remaining text or NULL.
 */
const char *qui_overflow(const qui_div_t *div);

/** @} */

/** @defgroup qui_layout QGL UI layout and rendering
 * @brief Applying styles, computing geometry and drawing.
 * @{
 */

/**
 * @brief Apply stylesheet rules and compute final styles.
 *
 * Recursively merges inherited and class-defined
 * styles throughout the element tree.
 */
void qui_apply_styles(qui_div_t *root, qui_style_rule_t *ss);

/**
 * @brief Compute layout positions and sizes.
 *
 * Calculates geometry for the subtree rooted at
 * @p root within the specified box.
 */
void qui_layout(qui_div_t *root);

/**
 * @brief Render the element tree.
 *
 * Draws backgrounds, borders and text recursively.
 */
void qui_render(qui_div_t *root);

/**
 * @brief Recursively free a UI tree.
 */
void qui_clear(qui_div_t *root);

/**
 * @brief Reset a style struct to its default values.
 *
 * @param[out] s Pointer to the style struct to reset.
 */
void qui_style_reset(qui_style_t *s);

/**
 * @brief Draw a rectangle with rounded corners and optional border.
 *
 * @param[in] background_color Fill color inside the rounded shape.
 * @param[in] border_color     Color of the border line.
 * @param[in] x,y              Top-left corner.
 * @param[in] w,h              Width and height in pixels.
 * @param[in] tl,tr,br,bl      Radii of each corner.
 * @param[in] border_width     Width of the border line.
 */
void qgl_border_radius(uint32_t background_color,
                       uint32_t border_color,
                       int32_t x, int32_t y,
                       uint32_t w, uint32_t h,
                       float tl, float tr,
                       float br, float bl,
                       float border_width);

/**
 * @brief Draw a blurred drop shadow behind a rounded box.
 *
 * Produces a CSS-like `box-shadow` effect using a Gaussian falloff.
 * The shadow is rendered as a translucent shape outside the box bounds,
 * respecting corner radii and blur radius.
 *
 * @param[in] color        Shadow color (BGRA).
 * @param[in] x,y          Top-left corner of the caster box.
 * @param[in] w,h          Width and height of the caster box.
 * @param[in] tl,tr,br,bl  Corner radii in pixels.
 * @param[in] blur_radius  Blur radius in pixels.
 * @param[in] offset_x     Horizontal offset of the shadow in pixels.
 * @param[in] offset_y     Vertical offset of the shadow in pixels.
 *
 * The function expands the shadow area to fully contain the blur and
 * offset. The actual rendering is clipped to the visible box area,
 * with smooth blending at the edges.
 */
void qgl_box_shadow(uint32_t color,
		    int32_t x, int32_t y,
		    uint32_t w, uint32_t h,
		    float tl, float tr, float br, float bl,
		    float blur_radius, float offset_x, float offset_y);

void qui_mark_dirty(qui_div_t *d);

void qui_style_set(qui_div_t *root, size_t offset,
		   const void *value, size_t size);

/** @} */

#endif /* QGL_UI_H */
