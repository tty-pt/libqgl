#ifndef QGL_CACHE_H
#define QGL_CACHE_H

#include <stdint.h>
#include <ttypt/qgl.h>
#include <ttypt/qgl-ui.h>
#include "./gl.h"

/*
 * QGL render cache
 *
 * Provides offscreen caching for div trees.
 * When a div and its descendants don't change
 * (style, layout, or children), the cached FBO/texture
 * is reused directly.
 */

typedef struct qgl_cache_entry {
	GLuint tex;
	uint32_t w, h;
	int dirty;
} qgl_cache_entry_t;

/* Allocate or reset cache for a given div tree */
qgl_cache_entry_t *qgl_cache_alloc(qui_div_t *root);

/* Free cache and GPU resources */
void qgl_cache_free(qgl_cache_entry_t *cache);

/* Mark cache dirty (style/layout/children changed) */
void qgl_cache_invalidate(qui_div_t *d);

/* Renders the given div tree to cache texture */
void qgl_cache_build(qui_div_t *d);

/* Draws cached texture to screen (if valid) */
void qgl_cache_draw(const qui_div_t *d, int32_t x, int32_t y);

/* Returns 1 if cache can be reused */
int qgl_cache_valid(const qui_div_t *d);

#endif /* QGL_CACHE_H */
