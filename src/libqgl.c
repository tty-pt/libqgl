#include "../include/ttypt/qgl.h"
#include "./gl.h"
#include "./be.h"
#include "./input.h"
#include <ttypt/qsys.h>
#include <ttypt/qmap.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

qgl_be_t qgl_be;
qgl_input_t qgl_input;

static GLuint g_fbo;
GLuint g_tex;

static uint32_t g_tex_map_hd;
uint32_t qgl_height, qgl_width;
screen_t screen;

typedef struct {
	GLuint id;
	uint32_t w, h;
} gl_tex_info_t;

typedef struct {
	uint32_t ref;
	int32_t x, y;
	uint32_t cx, cy, sw, sh, dw, dh, tint;
} hw_cmd_t;

static void gl_make_tex(GLuint *out, uint32_t w, uint32_t h)
{
	glGenTextures(1, out);
	glBindTexture(GL_TEXTURE_2D, *out);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		     GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

static void gl_fullscreen_quad(uint32_t w, uint32_t h)
{
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 1);
	glVertex2f(0, 0);
	glTexCoord2f(1, 1);
	glVertex2f(w, 0);
	glTexCoord2f(0, 0);
	glVertex2f(0, h);
	glTexCoord2f(1, 0);
	glVertex2f(w, h);
	glEnd();
}

static void gl_set_ortho(uint32_t w, uint32_t h)
{
	glViewport(0, 0, (GLint)w, (GLint)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, h, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

extern qgl_be_t __attribute__((weak)) qgl_fb;
extern qgl_be_t qgl_glfw;
extern qgl_input_t __attribute__((weak)) qgl_input_dev;
extern qgl_input_t qgl_input_glfw;

void gl_init(uint32_t *w_r, uint32_t *h_r)
{
	uint32_t w, h;

	qgl_be.init(w_r, h_r);
	w = *w_r;
	h = *h_r;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4ub(255, 255, 255, 255);
	glClearColor(0, 0, 0, 1);

	gl_make_tex(&g_tex, w, h);
	glGenFramebuffers(1, &g_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, g_tex, 0);

	CBUG(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
	     "FBO incomplete");

	gl_set_ortho(w, h);
	glClear(GL_COLOR_BUFFER_BIT);

	uint32_t qm_gl_tex = qmap_reg(sizeof(gl_tex_info_t));
	g_tex_map_hd = qmap_open(NULL, NULL, QM_HNDL, qm_gl_tex, 0xF, 0);
}

void img_load_all(void);
void qlfw_construct(void);
void __attribute__((weak)) fb_construct(void);
void input_glfw_construct(void);
void __attribute__((weak)) input_dev_construct(void);

__attribute__((constructor))
static void construct(void)
{
	img_load_all();
	qgl_width = 1024;
	qgl_height = 768;

#if defined(_WIN32) || defined(__APPLE__)
	qlfw_construct();
	qgl_be = qgl_glfw;
	qgl_input = qgl_input_glfw;
#else
	if (getenv("DISPLAY")) {
		qlfw_construct();
		input_glfw_construct();
		qgl_be = qgl_glfw;
		qgl_input = qgl_input_glfw;
	} else if (&qgl_fb) {
		fb_construct();
		input_dev_construct();
		qgl_be = qgl_fb;
		qgl_input = qgl_input_dev;
	} else {
		CBUG(1, "No backend supported\n");
	}
#endif

	gl_init(&qgl_width, &qgl_height);
	qgl_input.init(0);
}

void qgl_flush(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl_set_ortho(qgl_width, qgl_height);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, g_tex);
	gl_fullscreen_quad(qgl_width, qgl_height);

	qgl_be.flush();

	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	gl_set_ortho(qgl_width, qgl_height);
}

void qgl_size(uint32_t *w, uint32_t *h)
{
	*w = qgl_width;
	*h = qgl_height;
}

static void gl_deinit(void)
{
	const void *key, *val;
	uint32_t it;

	if (g_tex)
		glDeleteTextures(1, &g_tex);
	if (g_fbo)
		glDeleteFramebuffers(1, &g_fbo);

	it = qmap_iter(g_tex_map_hd, NULL, 0);
	while (qmap_next(&key, &val, it))
		glDeleteTextures(1, &((gl_tex_info_t *)val)->id);
	qmap_close(g_tex_map_hd);
}

void img_deinit(void);

__attribute__((destructor))
static void destructor(void)
{
	qgl_input.deinit();
	gl_deinit();
	qgl_be.deinit();
	img_deinit();
}

void qgl_tex_draw_x(uint32_t ref, int32_t x, int32_t y,
		    uint32_t cx, uint32_t cy, uint32_t sw, uint32_t sh,
		    uint32_t dw, uint32_t dh, uint32_t tint)
{
	const gl_tex_info_t *tex = qmap_get(g_tex_map_hd, &ref);
	if (!tex)
		return;

	float u0 = (float)cx / tex->w;
	float v0 = (float)cy / tex->h;
	float u1 = (float)(cx + sw) / tex->w;
	float v1 = (float)(cy + sh) / tex->h;

	float x0 = x, y0 = y;
	float x1 = x + dw, y1 = y + dh;

	uint8_t ta = (tint >> 24) & 0xFF;
	uint8_t tr = (tint >> 16) & 0xFF;
	uint8_t tg = (tint >> 8) & 0xFF;
	uint8_t tb = tint & 0xFF;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glColor4ub(tr, tg, tb, ta);
	glBindTexture(GL_TEXTURE_2D, tex->id);

	glBegin(GL_QUADS);
	glTexCoord2f(u0, v0);
	glVertex2f(x0, y0);
	glTexCoord2f(u1, v0);
	glVertex2f(x1, y0);
	glTexCoord2f(u1, v1);
	glVertex2f(x1, y1);
	glTexCoord2f(u0, v1);
	glVertex2f(x0, y1);
	glEnd();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4ub(255, 255, 255, 255);
	glDisable(GL_BLEND);
}

void qgl_tex_reg(uint32_t ref, uint8_t *data, uint32_t w, uint32_t h)
{
	gl_tex_info_t tex = { .w = w, .h = h };

	glGenTextures(1, &tex.id);
	glBindTexture(GL_TEXTURE_2D, tex.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		     GL_BGRA, GL_UNSIGNED_BYTE, data);

	qmap_put(g_tex_map_hd, &ref, &tex);
}

void qgl_tex_upd(uint32_t ref, uint32_t x, uint32_t y,
		 uint32_t w, uint32_t h, uint8_t *data)
{
	const gl_tex_info_t *t = qmap_get(g_tex_map_hd, &ref);
	if (!t)
		return;

	glBindTexture(GL_TEXTURE_2D, t->id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
			GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void qgl_tex_ureg(uint32_t ref)
{
	const gl_tex_info_t *t = qmap_get(g_tex_map_hd, &ref);

	if (t) {
		glDeleteTextures(1, &t->id);
		qmap_del(g_tex_map_hd, &ref);
	}
}

void qgl_poll(void)
{
	qgl_input.poll();
}

void qgl_fill(int32_t x, int32_t y,
	      uint32_t w, uint32_t h,
	      uint32_t color)
{
	uint8_t a = (color >> 24) & 0xFF;
	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);

	glColor4ub(r, g, b, a);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor4ub(255, 255, 255, 255);
}
