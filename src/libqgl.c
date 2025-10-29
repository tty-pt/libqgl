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

GLuint g_tex;
GLuint g_fbo, g_tex_cpu;
uint32_t g_tex_map_hd;

typedef struct {
	GLuint id;
	uint32_t w, h;
} gl_tex_info_t;

typedef struct {
	uint32_t ref;
	int32_t x, y;
	uint32_t cx, cy, sw, sh, dw, dh, tint;
} hw_cmd_t;

#define MAX_HW_CMDS 4096
static hw_cmd_t g_cmds[MAX_HW_CMDS];
static size_t g_ncmds = 0;

static void gl_make_tex(GLuint *out, uint32_t w, uint32_t h)
{
	glGenTextures(1, out);
	glBindTexture(GL_TEXTURE_2D, *out);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_T, GL_CLAMP);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
	             GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

void gl_fullscreen_quad(uint32_t w, uint32_t h)
{
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 1); glVertex2f(0, 0);
	glTexCoord2f(1, 1); glVertex2f(w, 0);
	glTexCoord2f(0, 0); glVertex2f(0, h);
	glTexCoord2f(1, 0); glVertex2f(w, h);
	glEnd();
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
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);

	glClearColor(0, 0, 0, 1);

	screen.channels = 4;
	screen.size = w * h;
	screen.canvas = calloc(screen.size, screen.channels);
	CBUG(!screen.canvas, "calloc canvas");

	gl_make_tex(&g_tex, w, h);
	gl_make_tex(&g_tex_cpu, w, h);

	glGenFramebuffers(1, &g_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, g_tex, 0);
	CBUG(glCheckFramebufferStatus(GL_FRAMEBUFFER)
			!= GL_FRAMEBUFFER_COMPLETE,
	     "FBO incomplete");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	uint32_t qm_gl_tex = qmap_reg(sizeof(gl_tex_info_t));
	g_tex_map_hd = qmap_open(NULL, NULL, QM_HNDL,
			qm_gl_tex, 0xF, 0);

	glActiveTexture(GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4ub(255, 255, 255, 255);
}

void img_load_all(void);

void qlfw_construct(void);
void __attribute__((weak)) fb_construct(void);

void input_glfw_construct(void);
void __attribute__((weak)) input_dev_construct(void);

__attribute__((constructor))
static void
construct(void) {
	img_load_all();
	qgl_width = 1024;
	qgl_height = 768;

#if defined(_WIN32) || defined(__APPLE__)
    /* Windows: força backend GLFW */
    qlfw_construct();
    qgl_be    = qgl_glfw;
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
	glBindTexture(GL_TEXTURE_2D, g_tex_cpu);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
	                qgl_width, qgl_height, GL_BGRA,
	                GL_UNSIGNED_BYTE, screen.canvas);

	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glViewport(0, 0, qgl_width, qgl_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, g_tex_cpu);
	gl_fullscreen_quad(qgl_width, qgl_height);

	if (!g_ncmds)
		goto hw_skip;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,
			GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV,
			GL_TEXTURE_ENV_MODE,
			GL_MODULATE);
	for (size_t i = 0; i < g_ncmds; i++) {
		const hw_cmd_t *c = &g_cmds[i];
		const gl_tex_info_t *tex = qmap_get(g_tex_map_hd, &c->ref);
		if (!tex) continue;

		uint8_t ta = c->tint >> 24;
		uint8_t tr = c->tint >> 16;
		uint8_t tg = c->tint >> 8;
		uint8_t tb = c->tint;

		glColor4ub(tr, tg, tb, ta);
		glBindTexture(GL_TEXTURE_2D, tex->id);

		float u0 = (float)c->cx / tex->w;
		float v0 = (float)c->cy / tex->h;
		float u1 = (float)(c->cx + c->sw) / tex->w;
		float v1 = (float)(c->cy + c->sh) / tex->h;

		float x0 = c->x, y0 = c->y;
		float x1 = x0 + c->dw, y1 = y0 + c->dh;

		glBegin(GL_QUADS);
		glTexCoord2f(u0, v0); glVertex2f(x0, y0);
		glTexCoord2f(u1, v0); glVertex2f(x1, y0);
		glTexCoord2f(u1, v1); glVertex2f(x1, y1);
		glTexCoord2f(u0, v1); glVertex2f(x0, y1);
		glEnd();
	}
	glDisable(GL_BLEND);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4ub(255, 255, 255, 255);
hw_skip:

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, qgl_width, qgl_height,
			GL_BGRA, GL_UNSIGNED_BYTE,
			screen.canvas);
	g_ncmds = 0;

	qgl_be.flush();
}

void qgl_size(uint32_t *w, uint32_t *h) {
	*w = qgl_width;
	*h = qgl_height;
}

static void gl_deinit(void)
{
	const void *key, *val;
	uint32_t it;

	glDeleteTextures(1, &g_tex);
	glDeleteTextures(1, &g_tex_cpu);
	glDeleteFramebuffers(1, &g_fbo);

	it = qmap_iter(g_tex_map_hd, NULL, 0);
	while (qmap_next(&key, &val, it))
		glDeleteTextures(1, &((gl_tex_info_t*)val)->id);
	qmap_close(g_tex_map_hd);

	free(screen.canvas);
	memset(&screen, 0, sizeof(screen));
}

void img_deinit(void);

__attribute__((destructor))
static void destructor(void) {
	qgl_input.deinit();
	gl_deinit();
	qgl_be.deinit();
	img_deinit();
}

void qgl_tex_draw_x(uint32_t ref, int32_t x, int32_t y,
                 uint32_t cx, uint32_t cy, uint32_t sw, uint32_t sh,
                 uint32_t dw, uint32_t dh, uint32_t tint)
{
	if (g_ncmds < MAX_HW_CMDS)
		g_cmds[g_ncmds++] = (hw_cmd_t) {
			ref, x, y, cx, cy,
			sw, sh, dw, dh, tint
		};
}

void qgl_tex_reg(uint32_t ref, uint8_t *data, uint32_t w, uint32_t h)
{
	gl_tex_info_t tex = { .w = w, .h = h };
	glGenTextures(1, &tex.id);
	glBindTexture(GL_TEXTURE_2D, tex.id);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0,
			GL_RGBA8, w, h, 0, GL_BGRA,
			GL_UNSIGNED_BYTE, data);
	qmap_put(g_tex_map_hd, &ref, &tex);
}

void qgl_tex_upd(uint32_t ref, uint32_t x, uint32_t y,
                  uint32_t w, uint32_t h, uint8_t *data)
{
	const gl_tex_info_t *t = qmap_get(g_tex_map_hd, &ref);
	if (!t) return;
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

void qgl_poll(void) {
	qgl_input.poll();
}
