#include "./../include/ttypt/qgl.h"
#include "./gl.h"

#include <ttypt/qmap.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* cache key: already-colored RGBA texture */
struct key_round_color {
	uint32_t w, h;
	float tl, tr, br, bl;
	float width;
	uint32_t fill_rgba;
	uint32_t stroke_rgba;
};

static uint32_t g_round_tex_map_hd;

/* external state from core */
extern GLuint g_fbo, g_vao_dummy, g_prog_tex;
extern GLint g_uDst_tex, g_uUV_tex, g_uTint_tex;
extern float qgl_ortho_M[16];

/* fill fragment shader */
static const char *FS_FILL_ROUND =
"#version 330 core\n"
"in vec2 vPos;\n"
"uniform vec4 uColor;\n"
"uniform vec4 uDst;\n"
"uniform vec4 uRadius;\n"
"out vec4 FragColor;\n"
"float sdRoundedBox(vec2 p, vec2 b, vec4 r){\n"
"	float m = 0.5 * min(b.x, b.y);\n"
"	r = clamp(r, 0.0, m);\n"
"	vec2 pc = p - b * 0.5;\n"
"	vec2 bh = b * 0.5;\n"
"	float r_x = (pc.x > 0.0) ? r.y : r.x;\n"
"	float r_y = (pc.x > 0.0) ? r.z : r.w;\n"
"	float rq  = (pc.y > 0.0) ? r_x : r_y;\n"
"	vec2 q = abs(pc) - bh + rq;\n"
"	return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - rq;\n"
"}\n"
"void main(){\n"
"	vec2 size = uDst.zw;\n"
"	float d = sdRoundedBox(vPos, size, uRadius);\n"
"	float aa = fwidth(d);\n"
"	float coverage = clamp(0.5 - d/aa, 0.0, 1.0);\n"
"	if (coverage <= 0.0) discard;\n"
"	FragColor = vec4(uColor.rgb, uColor.a * coverage);\n"
"}\n";

/* stroke fragment shader */
static const char *FS_STROKE_ROUND =
"#version 330 core\n"
"in vec2 vPos;\n"
"uniform vec4 uColor;\n"
"uniform vec4 uDst;\n"
"uniform vec4 uRadius;\n"
"uniform float uWidth;\n"
"out vec4 FragColor;\n"
"float sdRoundedBox(vec2 p, vec2 b, vec4 r){\n"
"	float m = 0.5 * min(b.x, b.y);\n"
"	r = clamp(r, 0.0, m);\n"
"	vec2 pc = p - b * 0.5;\n"
"	vec2 bh = b * 0.5;\n"
"	float r_x = (pc.x > 0.0) ? r.y : r.x;\n"
"	float r_y = (pc.x > 0.0) ? r.z : r.w;\n"
"	float rq  = (pc.y > 0.0) ? r_x : r_y;\n"
"	vec2 q = abs(pc) - bh + rq;\n"
"	return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - rq;\n"
"}\n"
"void main(){\n"
"	vec2 size = uDst.zw;\n"
"	float w = uWidth * 1.25;\n"
"	float dOuter = sdRoundedBox(vPos, size, uRadius);\n"
"	float dInner = sdRoundedBox(vPos - vec2(w, w), size - 2.0 * vec2(w), max(uRadius - w, 0.0));\n"
"	if (dOuter > 0.0 || dInner < 0.0) discard;\n"
"	float aa = fwidth(dOuter);\n"
"	float aOuter = smoothstep(0.0, aa, -dOuter);\n"
"	float aInner = smoothstep(0.0, aa,  dInner);\n"
"	float alpha = aOuter * aInner;\n"
"	FragColor = vec4(uColor.rgb, uColor.a * alpha);\n"
"}\n";

/* GLSL programs (offscreen) */
static GLuint prog_fill_round, prog_stroke_round;
static GLint uProj_fill, uDst_fill, uColor_fill, uRadius_fill;
static GLint uProj_stroke, uDst_stroke, uColor_stroke, uRadius_stroke, uWidth_stroke;

static void ortho_local(float *m, float w, float h)
{
	memset(m, 0, sizeof(float) * 16);
	m[0] = 2.0f / w;
	m[5] = -2.0f / h;
	m[10] = -1.0f;
	m[12] = -1.0f;
	m[13] = 1.0f;
	m[15] = 1.0f;
}

static inline void rgba_u32_to_f4(uint32_t c, float out[4])
{
	out[3] = ((c >> 24) & 0xFF) / 255.0f; // a
	out[0] = ((c >> 16) & 0xFF) / 255.0f; // r
	out[1] = ((c >>  8) & 0xFF) / 255.0f; // g
	out[2] = ((c      ) & 0xFF) / 255.0f; // b
}

static GLuint make_colored_round_tex(uint32_t w, uint32_t h,
				     uint32_t fill_rgba, uint32_t stroke_rgba,
				     float tl, float tr, float br, float bl,
				     float border_width)
{
	GLuint tex, fbo;
	float m[16];
	float dst[4], radius[4], color[4];

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);

	glViewport(0, 0, (GLint)w, (GLint)h);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	ortho_local(m, (float)w, (float)h);

	/* fill */
	if (fill_rgba & 0xff000000u) {
		dst[0] = 0; dst[1] = 0;
		dst[2] = w; dst[3] = h;
		radius[0] = tl; radius[1] = tr;
		radius[2] = br; radius[3] = bl;
		rgba_u32_to_f4(fill_rgba, color);

		glUseProgram(prog_fill_round);
		glUniformMatrix4fv(uProj_fill, 1, GL_FALSE, m);
		glUniform4fv(uDst_fill, 1, dst);
		glUniform4fv(uRadius_fill, 1, radius);
		glUniform4fv(uColor_fill, 1, color);
		glBindVertexArray(g_vao_dummy);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	/* stroke */
	if (border_width > 0.0f && (stroke_rgba & 0xff000000u)) {
		dst[0] = 0; dst[1] = 0;
		dst[2] = w; dst[3] = h;
		radius[0] = tl; radius[1] = tr;
		radius[2] = br; radius[3] = bl;
		rgba_u32_to_f4(stroke_rgba, color);

		glUseProgram(prog_stroke_round);
		glUniformMatrix4fv(uProj_stroke, 1, GL_FALSE, m);
		glUniform4fv(uDst_stroke, 1, dst);
		glUniform4fv(uRadius_stroke, 1, radius);
		glUniform1f(uWidth_stroke, border_width);
		glUniform4fv(uColor_stroke, 1, color);
		glBindVertexArray(g_vao_dummy);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	GLint prev_fbo = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);

	/* restore whatever framebuffer was bound before */
	glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
	glDeleteFramebuffers(1, &fbo);

	return tex;
}

static GLuint get_colored_round_tex(uint32_t w, uint32_t h,
				    float tl, float tr, float br, float bl,
				    float border_width,
				    uint32_t fill_rgba, uint32_t stroke_rgba)
{
	struct key_round_color k = {
		.w = w, .h = h,
		.tl = tl, .tr = tr, .br = br, .bl = bl,
		.width = border_width,
		.fill_rgba = fill_rgba,
		.stroke_rgba = stroke_rgba,
	};
	const GLuint *pt;
	GLuint tex;

	pt = qmap_get(g_round_tex_map_hd, &k);
	if (pt)
		return *pt;

	tex = make_colored_round_tex(w, h, fill_rgba, stroke_rgba,
				     tl, tr, br, bl, border_width);
	qmap_put(g_round_tex_map_hd, &k, &tex);
	return tex;
}

void qgl_border_radius(uint32_t bg_color, uint32_t border_color,
		       int32_t x, int32_t y, uint32_t w, uint32_t h,
		       float tl, float tr, float br, float bl,
		       float border_width)
{
	GLuint tex;
	float dst[4], uv[4], tint[4] = {1, 1, 1, 1};

	tex = get_colored_round_tex(w, h, tl, tr, br, bl,
				    border_width, bg_color, border_color);

	dst[0] = x; dst[1] = y; dst[2] = w; dst[3] = h;
	uv[0] = 0; uv[1] = 0; uv[2] = 1; uv[3] = 1;

	glUseProgram(g_prog_tex);
	glBindVertexArray(g_vao_dummy);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform4fv(g_uDst_tex, 1, dst);
	glUniform4fv(g_uUV_tex, 1, uv);
	glUniform4fv(g_uTint_tex, 1, tint);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void shadow_init(void)
{
	uint32_t qm_gl_tex;

	qm_gl_tex = qmap_reg(sizeof(GLuint));
	g_round_tex_map_hd = qmap_open(NULL, NULL, QM_HNDL, qm_gl_tex, 0xf, 0);

	prog_fill_round = qgl_link(
		qgl_compile(GL_VERTEX_SHADER, VS_FILL),
		qgl_compile(GL_FRAGMENT_SHADER, FS_FILL_ROUND));
	uProj_fill = glGetUniformLocation(prog_fill_round, "uProj");
	uDst_fill = glGetUniformLocation(prog_fill_round, "uDst");
	uColor_fill = glGetUniformLocation(prog_fill_round, "uColor");
	uRadius_fill = glGetUniformLocation(prog_fill_round, "uRadius");

	prog_stroke_round = qgl_link(
		qgl_compile(GL_VERTEX_SHADER, VS_FILL),
		qgl_compile(GL_FRAGMENT_SHADER, FS_STROKE_ROUND));
	uProj_stroke = glGetUniformLocation(prog_stroke_round, "uProj");
	uDst_stroke = glGetUniformLocation(prog_stroke_round, "uDst");
	uColor_stroke = glGetUniformLocation(prog_stroke_round, "uColor");
	uRadius_stroke = glGetUniformLocation(prog_stroke_round, "uRadius");
	uWidth_stroke = glGetUniformLocation(prog_stroke_round, "uWidth");
}

void shadow_deinit(void)
{
	const void *key, *val;
	uint32_t it;

	it = qmap_iter(g_round_tex_map_hd, NULL, 0);
	while (qmap_next(&key, &val, it))
		glDeleteTextures(1, (const GLuint *)val);
	qmap_close(g_round_tex_map_hd);

	glDeleteProgram(prog_fill_round);
	glDeleteProgram(prog_stroke_round);
}
