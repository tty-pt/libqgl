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
#include "qgl-font.h"
#include <stdint.h>

/** @defgroup qui_types QGL UI types
 *  @brief Fundamental type definitions.
 *  @{
 */

/**
 * @brief Layout direction for container elements.
 */
typedef enum {
	UI_ROW,
	UI_COLUMN
} qui_flex_dir_t;

/**
 * @brief Positioning mode for elements.
 */
typedef enum {
	UI_POS_RELATIVE = 0,
	UI_POS_ABSOLUTE = 1
} qui_position_t;

/**
 * @brief Display mode for visibility control.
 */
typedef enum {
	UI_DISPLAY_BLOCK = 0,
	UI_DISPLAY_NONE  = 1
} qui_display_t;

/**
 * @brief Visual style definition for a UI element.
 *
 * Represents colors, borders, fonts and layout
 * metrics. Styles may be defined globally in a
 * stylesheet and applied by class name.
 */
typedef struct {
	uint32_t bg_color;      /**< Background color (BGRA). */
	uint32_t border_color;  /**< Border color (BGRA). */
	uint32_t border_size;   /**< Border thickness in pixels. */
	uint32_t bg_tex;        /**< Background texture reference (QM_MISS = none). */
	uint32_t font_ref;      /**< Font reference (QM_MISS = none). */
	uint32_t font_scale;    /**< Text scale factor. */
	uint32_t text_color;    /**< Text color (BGRA). */
	uint8_t  align;         /**< 0=left, 1=center, 2=right. */

	uint32_t pad_left;
	uint32_t pad_right;
	uint32_t pad_top;
	uint32_t pad_bottom;

	/* positioning */
	qui_position_t position;
	int32_t  top;
	int32_t  left;
	int32_t  right;
	int32_t  bottom;

	qui_display_t  display;

	/* flexbox layout */
	qui_flex_dir_t dir;     /**< UI_ROW or UI_COLUMN. */
	float grow;             /**< Flex grow factor. */
	float shrink;           /**< Flex shrink factor. */
	uint32_t basis;         /**< Flex basis (pixels). */
} qui_style_t;

/**
 * @brief A single stylesheet rule.
 */
typedef struct qui_style_rule {
	const char *class_name;
	qui_style_t style;
	struct qui_style_rule *next;
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
 *  @brief Core setup and stylesheet management.
 *  @{
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
 *  @brief Element creation and configuration.
 *  @{
 */

/**
 * @brief Create a new UI element.
 *
 * @param[in] parent Parent div, or NULL for root.
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
 *  @brief Applying styles, computing geometry and drawing.
 *  @{
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
void qui_layout(qui_div_t *root,
		int32_t x,
		int32_t y,
		uint32_t w,
		uint32_t h);

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

/** @} */

#endif /* QGL_UI_H */
