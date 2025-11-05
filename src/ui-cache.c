#include "./gl.h"
#include "./ui.h"
#include <string.h>
#include <stdlib.h>
#include <xxhash.h>

extern GLuint g_fbo, g_vao_dummy, g_prog_tex;
extern GLint g_uDst_tex, g_uUV_tex, g_uTint_tex;

/* Each div optionally owns a cache */
typedef struct {
	GLuint		tex;
	uint32_t	w, h;
	int		dirty;
	uint64_t	hash;
} qgl_cache_t;

/* Attach to qui_div via its style->cache or extended struct in your engine */
#define DIV_CACHE(d)	((qgl_cache_t *)((d)->_cache))

void qgl_cache_init(void)
{
	/* nothing global for now */
}

void qgl_cache_deinit(void)
{
	/* nothing */
}

uint64_t qgl_cache_hash_style(const qui_style_t *s, uint32_t w, uint32_t h)
{
	XXH64_state_t *state = XXH64_createState();
	XXH64_reset(state, 0);
	XXH64_update(state, s, sizeof(*s));
	XXH64_update(state, &w, sizeof(w));
	XXH64_update(state, &h, sizeof(h));
	uint64_t h64 = XXH64_digest(state);
	XXH64_freeState(state);
	return h64;
}

void qgl_cache_invalidate(qui_div_t *d)
{
	if (!d || !DIV_CACHE(d))
		return;

	DIV_CACHE(d)->dirty = 1;
}

void qgl_cache_invalidate_tree(qui_div_t *root)
{
	if (!root)
		return;

	qgl_cache_invalidate(root);
	for (qui_div_t *c = root->first_child; c; c = c->next_sibling)
		qgl_cache_invalidate_tree(c);
}

int qgl_cache_valid(const qui_div_t *d)
{
	qgl_cache_t *c = DIV_CACHE(d);

	if (!c)
		return 0;
	if (c->dirty)
		return 0;
	if (!c->tex)		/* ← novo */
		return 0;
	if (c->w != d->w || c->h != d->h)
		return 0;

	return 1;
}

/* build the cache (offscreen render of subtree) */
void qgl_cache_build(qui_div_t *d)
{
	qgl_cache_t *c = DIV_CACHE(d);

	if (!c)
		d->_cache = c = calloc(1, sizeof(*c));

	if (d->w == 0 || d->h == 0) {
		c->dirty = 1;
		return;
	}

	uint64_t hash = qgl_cache_hash_style(d->style, d->w, d->h);

	if (c->tex)
		glDeleteTextures(1, &c->tex);

	c->w = d->w;
	c->h = d->h;
	c->hash = hash;

	GLuint tex, fbo;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, d->w, d->h, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &fbo);

	/* guardar estado atual e ir para o FBO do cache */
	GLint prev_fbo;
	GLint vp[4];

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
	glGetIntegerv(GL_VIEWPORT, vp);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex, 0);
	glViewport(0, 0, (GLint)d->w, (GLint)d->h);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* forçar caminho sem cache durante a construção */
	int old_dirty = c->dirty;

	c->dirty = 1;		/* ← crítico: impede o early-return em render_div() */
	render_div_raw(d);
	c->dirty = old_dirty;	/* repor flag local (vamos marcar 0 já a seguir) */

	/* restaurar FBO/viewport anteriores */
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prev_fbo);
	glViewport(vp[0], vp[1], vp[2], vp[3]);
	glDeleteFramebuffers(1, &fbo);

	c->tex = tex;
	c->dirty = 0;		/* ← só agora fica válido */
}

/* draw cached image */
void qgl_cache_draw(const qui_div_t *d, int32_t x, int32_t y)
{
	const qgl_cache_t *c = DIV_CACHE(d);
	float dst[4]  = { (float)x, (float)y, (float)c->w, (float)c->h };
	float uv[4]   = { 0, 0, 1, 1 };
	float tint[4] = { 1, 1, 1, 1 };

	if (!c || !c->tex)
		return;

	glUseProgram(g_prog_tex);
	glBindVertexArray(g_vao_dummy);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, c->tex);
	glUniform4fv(g_uDst_tex, 1, dst);
	glUniform4fv(g_uUV_tex, 1, uv);
	glUniform4fv(g_uTint_tex, 1, tint);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
