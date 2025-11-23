/**
 * @file gl.h
 * @brief Internal OpenGL utility layer for the QGL engine.
 *
 * Provides minimal wrappers for shader compilation, linking,
 * orthographic projection setup, and framebuffer/viewport management.
 * This header is **not** part of the public API; it is used by
 * QGL’s renderer, cache system, and backend glue code.
 */

#ifndef QGL_GL_H
#define QGL_GL_H

#define GL_GLEXT_PROTOTYPES 1
#define GLEW_STATIC

#ifdef __APPLE__
# define GL_SILENCE_DEPRECATION
# include <OpenGL/glext.h>
# include <OpenGL/gl3.h>
#else
# include <GL/glew.h>
# include <GL/gl.h>
#endif

/** Off-screen framebuffer object used as the engine’s main render target. */
extern GLuint g_fbo;

/** Dummy vertex array (required by core profile). */
extern GLuint g_vao_dummy;

/** Uniform locations for the texture drawing program. */
extern GLint g_uProj_tex;   /**< Projection matrix uniform. */
extern GLint g_uDst_tex;    /**< Destination rectangle (x,y,w,h). */
extern GLint g_uUV_tex;     /**< Texture UV coordinates. */
extern GLint g_uTint_tex;   /**< RGBA tint multiplier. */
extern GLint g_uSampler;    /**< Sampler unit (texture binding index). */

/** Active texture drawing shader program. */
extern GLuint g_prog_tex;

/** Global orthographic projection matrix. */
extern float qgl_ortho_M[16];

/** Vertex shader source for solid-color fills. */
extern const char *VS_FILL;

/**
 * @brief Compute an orthographic projection matrix.
 *
 * Produces a column-major matrix that maps (0,0)–(w,h) to clip space.
 *
 * @param w Width of the projection volume.
 * @param h Height of the projection volume.
 * @param m Output 4×4 matrix (16 floats).
 */
void qgl_ortho(float w, float h, float *m);

/**
 * @brief Link an OpenGL shader program from compiled vertex and fragment shaders.
 *
 * @param vs Compiled vertex shader ID.
 * @param fs Compiled fragment shader ID.
 * @return Linked program ID.
 */
GLuint qgl_link(GLuint vs, GLuint fs);

/**
 * @brief Compile a shader from GLSL source.
 *
 * Exits on error, printing the shader log to stderr.
 *
 * @param type GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 * @param src  GLSL source string.
 * @return Shader object ID.
 */
GLuint qgl_compile(GLenum type, const char *src);

/**
 * @brief Set the current render target and viewport.
 *
 * Binds the given FBO and updates internal viewport state
 * used by `qgl_apply_ortho()`.
 *
 * @param fbo Framebuffer object ID.
 * @param w   Width of the viewport in pixels.
 * @param h   Height of the viewport in pixels.
 */
void qgl_set_viewport(GLuint fbo, uint32_t w, uint32_t h);

/**
 * @brief Restore the default engine framebuffer and viewport.
 *
 * Rebinds the main engine FBO (`g_fbo`) and resets
 * `g_view_w/h` to the screen size.
 */
void qgl_reset_viewport(void);

/**
 * @brief Upload the current orthographic matrix to a shader uniform.
 *
 * Uses the active `g_view_w/h` dimensions to build a matching
 * projection and send it to the specified uniform.
 *
 * @param uProj Location of the `uProj` uniform in the current program.
 */
void qgl_apply_ortho(GLint uProj);

/**
 * @brief Query the currently bound framebuffer object.
 *
 * @return The OpenGL ID of the active framebuffer.
 */
GLuint qgl_current_fbo(void);

#endif /* QGL_GL_H */
