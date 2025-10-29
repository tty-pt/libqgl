#ifndef QGL_TEX_H
#define QGL_TEX_H

#include <stdint.h>

enum img_new_flags {
	IMG_LOAD,
};

typedef unsigned img_load_t(const char *filename);
typedef int img_save_t(const char *filename,
		const uint8_t *data, uint32_t w, uint32_t h);

typedef struct img_be {
	img_load_t *load;
	img_save_t *save;
} img_be_t;

void img_be_load(char *ext, img_load_t *load, img_save_t *save);

unsigned img_new(uint8_t **data,
		const char *filename,
		uint32_t w, uint32_t h,
		unsigned flags);

void qgl_tex_reg(uint32_t ref, uint8_t *data,
			 uint32_t w, uint32_t h);

void qgl_tex_ureg(uint32_t ref);

void qgl_tex_upd(uint32_t ref, uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, uint8_t *data);

#endif
