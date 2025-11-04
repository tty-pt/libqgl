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

static GLuint g_prog_tex, g_prog_fill;
static GLint  g_uProj_tex, g_uDst_tex,
	      g_uUV_tex, g_uTint_tex, g_uSampler;
static GLint  g_uProj_fill, g_uDst_fill, g_uColor_fill;

static GLuint g_vao_dummy;

static uint32_t g_tex_map_hd;
uint32_t qgl_height, qgl_width;
screen_t screen;

static const char *VS_TEX = "#version 330 core\n"
"uniform mat4 uProj;\n"
"uniform vec4 uDst;  // x,y,w,h em pixels\n"
"uniform vec4 uUV;   // u0,v0,u1,v1\n"
"out vec2 vUV;\n"
"void main(){\n"
"  // 0:(0,0) 1:(1,0) 2:(1,1) 3:(0,1)\n"
"  int id = gl_VertexID;\n"
"  vec2 p = vec2((id==1||id==2)?1.0:0.0, (id>=2)?1.0:0.0);\n"
"  vec2 pos = uDst.xy + p * uDst.zw;\n"
"  gl_Position = uProj * vec4(pos, 0.0, 1.0);\n"
"  vec2 uv0 = uUV.xy, uv1 = uUV.zw;\n"
"  vUV = mix(uv0, uv1, p);\n"
"}\n";

static const char *FS_TEX = "#version 330 core\n"
"in vec2 vUV;\n"
"uniform sampler2D uTex;\n"
"uniform vec4 uTint; // RGBA 0..1\n"
"out vec4 FragColor;\n"
"void main(){ FragColor = texture(uTex, vUV) * uTint; }\n";

static const char *VS_FILL = "#version 330 core\n"
"uniform mat4 uProj;\n"
"uniform vec4 uDst; // x,y,w,h\n"
"void main(){\n"
"  int id = gl_VertexID;\n"
"  vec2 p = vec2((id==1||id==2)?1.0:0.0, (id>=2)?1.0:0.0);\n"
"  vec2 pos = uDst.xy + p * uDst.zw;\n"
"  gl_Position = uProj * vec4(pos, 0.0, 1.0);\n"
"}\n";

static const char *FS_FILL = "#version 330 core\n"
"uniform vec4 uColor;\n"
"out vec4 FragColor;\n"
"void main(){ FragColor = uColor; }\n";

typedef struct {
	GLuint id;
	uint32_t w, h;
} gl_tex_info_t;

typedef struct {
	uint32_t ref;
	int32_t x, y;
	uint32_t cx, cy, sw, sh, dw, dh, tint;
} hw_cmd_t;

static GLuint compile(GLenum type, const char *src) {
	GLuint s = glCreateShader(type);
	glShaderSource(s, 1, &src, NULL);
	glCompileShader(s);
	GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) { char log[4096]; glGetShaderInfoLog(s, sizeof log, NULL, log); fprintf(stderr,"shader: %s\n", log); exit(1); }
	return s;
}

static GLuint link(GLuint vs, GLuint fs) {
	GLuint p = glCreateProgram();
	glAttachShader(p, vs); glAttachShader(p, fs);
	glLinkProgram(p);
	GLint ok=0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if (!ok){ char log[4096]; glGetProgramInfoLog(p, sizeof log, NULL, log); fprintf(stderr,"link: %s\n", log); exit(1); }
	glDetachShader(p, vs); glDetachShader(p, fs);
	glDeleteShader(vs); glDeleteShader(fs);
	return p;
}

static void ortho(float w, float h, float *m) {
	// matriz column-major: ortho(0,w,0,h,-1,1)
	memset(m, 0, 16 * sizeof(float));
	m[0]  =  2.0f / w;
	m[5]  =  2.0f / h;
	m[10] = -1.0f;
	m[12] = -1.0f;
	m[13] = -1.0f;
	m[15] =  1.0f;
}

extern qgl_be_t __attribute__((weak)) qgl_fb;
extern qgl_be_t qgl_glfw;
extern qgl_input_t __attribute__((weak)) qgl_input_dev;
extern qgl_input_t qgl_input_glfw;

void gl_init(uint32_t *w_r, uint32_t *h_r)
{
	uint32_t w, h;

	qgl_be.init(w_r, h_r);
	w = *w_r; h = *h_r;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// VAO obrigatório em core profile
	glGenVertexArrays(1, &g_vao_dummy);
	glBindVertexArray(g_vao_dummy);

	// FBO com texture alvo (framebuffer “virtual” do teu engine)
	glGenTextures(1, &g_tex);
	glBindTexture(GL_TEXTURE_2D, g_tex);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &g_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, g_tex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,"FBO incomplete\n");
		exit(1);
	}

	// Shaders
	g_prog_tex  = link(compile(GL_VERTEX_SHADER, VS_TEX),
			compile(GL_FRAGMENT_SHADER, FS_TEX));
	g_prog_fill = link(compile(GL_VERTEX_SHADER, VS_FILL),
			compile(GL_FRAGMENT_SHADER, FS_FILL));

	// Locais
	glUseProgram(g_prog_tex);
	g_uProj_tex = glGetUniformLocation(g_prog_tex, "uProj");
	g_uDst_tex  = glGetUniformLocation(g_prog_tex, "uDst");
	g_uUV_tex   = glGetUniformLocation(g_prog_tex, "uUV");
	g_uTint_tex = glGetUniformLocation(g_prog_tex, "uTint");
	g_uSampler  = glGetUniformLocation(g_prog_tex, "uTex");
	glUniform1i(g_uSampler, 0); // texture unit 0

	glUseProgram(g_prog_fill);
	g_uProj_fill = glGetUniformLocation(g_prog_fill, "uProj");
	g_uDst_fill  = glGetUniformLocation(g_prog_fill, "uDst");
	g_uColor_fill= glGetUniformLocation(g_prog_fill, "uColor");

	// Projeção inicial
	float M[16];
	ortho((float)w, (float)h, M);

	glUseProgram(g_prog_tex);
	glUniformMatrix4fv(g_uProj_tex,  1, GL_FALSE, M);

	glUseProgram(g_prog_fill);
	glUniformMatrix4fv(g_uProj_fill, 1, GL_FALSE, M);

	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glViewport(0, 0, (GLint)w, (GLint)h);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);

	// assets CPU side
	screen.channels = 4;
	screen.size = w * h;
	screen.canvas = calloc(screen.size, screen.channels);
	CBUG(!screen.canvas, "calloc canvas");

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
	// update reading FBO -> screen.canvas
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, qgl_width, qgl_height, GL_BGRA, GL_UNSIGNED_BYTE, screen.canvas);

	// deliver to backend (that swaps buffers)
	qgl_be.flush();

	// clean FBO
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glViewport(0, 0, (GLint)qgl_width, (GLint)qgl_height);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
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

	glDeleteTextures(1, &g_tex);
	glDeleteFramebuffers(1, &g_fbo);

	it = qmap_iter(g_tex_map_hd, NULL, 0);
	while (qmap_next(&key, &val, it))
		glDeleteTextures(1, &((gl_tex_info_t *)val)->id);
	qmap_close(g_tex_map_hd);
	free(screen.canvas);
	memset(&screen, 0, sizeof(screen));
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
	if (!tex) return;

	float u0 = (float)cx / (float)tex->w;
	float v0 = (float)cy / (float)tex->h;
	float u1 = (float)(cx + sw) / (float)tex->w;
	float v1 = (float)(cy + sh) / (float)tex->h;

	float dst[4] = { (float)x, (float)y, (float)dw, (float)dh };
	float uv [4] = { u0, v0, u1, v1 };

	float ta = ((tint >> 24) & 0xFF) / 255.0f;
	float tr = ((tint >> 16) & 0xFF) / 255.0f;
	float tg = ((tint >>  8) & 0xFF) / 255.0f;
	float tb = ((tint      ) & 0xFF) / 255.0f;
	float rgba[4] = { tr, tg, tb, ta };

	glUseProgram(g_prog_tex);
	glBindVertexArray(g_vao_dummy);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->id);

	glUniform4fv(g_uDst_tex, 1, dst);
	glUniform4fv(g_uUV_tex,  1, uv);
	glUniform4fv(g_uTint_tex,1, rgba);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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

void qgl_fill(int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color)
{
	float a = ((color >> 24) & 0xFF) / 255.0f;
	float r = ((color >> 16) & 0xFF) / 255.0f;
	float g = ((color >>  8) & 0xFF) / 255.0f;
	float b = ((color      ) & 0xFF) / 255.0f;

	float dst[4]   = { (float)x, (float)y, (float)w, (float)h };
	float rgba[4]  = { r, g, b, a };

	glUseProgram(g_prog_fill);
	glBindVertexArray(g_vao_dummy);
	glUniform4fv(g_uDst_fill,   1, dst);
	glUniform4fv(g_uColor_fill, 1, rgba);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
