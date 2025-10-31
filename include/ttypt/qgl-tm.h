#ifndef QGL_TILE_H
#define QGL_TILE_H

/**
 * @file qgl-tile.h
 * @brief Public header for the QGL tilemap system.
 *
 * Declares the API for managing and rendering
 * 2D tilemaps in the QGL rendering layer.
 * Provides functions to register, query and
 * draw tiles from image atlases.
 */

#include <stdint.h>

/** @defgroup qgl_tile_types QGL tilemap types
 *  @brief Type definitions for tilemap handling.
 *  @{
 */

/**
 * @brief Tilemap descriptor.
 *
 * Represents a subdivided texture atlas used
 * for rendering multiple tiles from a single
 * image reference.
 */
typedef struct {
	unsigned img;  /**< Image reference (QGL texture ID). */
	uint32_t w;    /**< Tile width in pixels. */
	uint32_t h;    /**< Tile height in pixels. */
	uint32_t nx;   /**< Number of tiles per row. */
	uint32_t ny;   /**< Number of tiles per column. */
} qgl_tm_t;

/** @} */

/** @defgroup qgl_tile_core QGL tilemap creation and access
 *  @brief Tilemap management functions.
 *  @{
 */

/**
 * @brief Register a new tilemap.
 *
 * Divides the given image into fixed-size tiles.
 *
 * @param[in] img_ref Image reference (QGL texture ID).
 * @param[in] w       Tile width.
 * @param[in] h       Tile height.
 * @return            Tilemap handle (tm_ref).
 */
uint32_t qgl_tm_new(uint32_t img_ref,
		    uint32_t w,
		    uint32_t h);

/**
 * @brief Retrieve a tilemap descriptor.
 *
 * @param[in] tm_ref Tilemap handle.
 * @return           Pointer to descriptor, or NULL if invalid.
 */
const qgl_tm_t *qgl_tm_get(uint32_t tm_ref);

/** @} */

/** @defgroup qgl_tile_render QGL tile rendering
 *  @brief Functions for drawing individual tiles.
 *  @{
 */

/**
 * @brief Draw a single tile from a tilemap.
 *
 * @param[in] ref Tilemap handle.
 * @param[in] idx Tile index (0-based).
 * @param[in] x   Destination X position.
 * @param[in] y   Destination Y position.
 * @param[in] w   Destination width.
 * @param[in] h   Destination height.
 * @param[in] rx  Optional crop offset X (source space).
 * @param[in] ry  Optional crop offset Y (source space).
 */
void qgl_tile_draw(uint32_t ref,
		   uint32_t idx,
		   uint32_t x,
		   uint32_t y,
		   uint32_t w,
		   uint32_t h,
		   uint32_t rx,
		   uint32_t ry);

/** @} */

#endif /* QGL_TILE_H */
