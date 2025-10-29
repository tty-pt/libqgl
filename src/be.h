#ifndef QGL_BE_H
#define QGL_BE_H

#include <stdint.h>

typedef struct {
	uint32_t size;
	uint8_t *canvas;
	uint8_t channels;
	uint32_t min_x, min_y, max_x, max_y;
} screen_t;

extern screen_t screen;

typedef void qgl_be_init_t(uint32_t *w, uint32_t *h);
typedef void qgl_be_deinit_t(void);

typedef struct {
	qgl_be_init_t *init;
	qgl_be_deinit_t *flush, *deinit;
} qgl_be_t;

extern uint32_t qgl_width, qgl_height;

#endif
