#include "./gl.h"
#include "./be.h"

#include <stdlib.h>

#include <GLFW/glfw3.h>

#include <ttypt/qsys.h>

qgl_be_t qgl_glfw;

static GLuint g_prog_present, g_vao_present;

GLFWwindow *g_win;

extern GLuint g_tex;

static const char *VS_PRESENT = "#version 330 core\n"
"out vec2 vUV;\n"
"void main(){\n"
"  vec2 p = vec2((gl_VertexID==1||gl_VertexID==2)?1.0:0.0,\n"
"                (gl_VertexID>=2)?1.0:0.0);\n"
"  vUV = vec2(p.x, 1.0 - p.y);\n"  // flip Y se quiseres
"  gl_Position = vec4(p*2.0-1.0, 0.0, 1.0);\n"
"}\n";

static const char *FS_PRESENT = "#version 330 core\n"
"in vec2 vUV;\n"
"uniform sampler2D uTex;\n"
"out vec4 FragColor;\n"
"void main(){ FragColor = texture(uTex, vUV); }\n";

static void glfw_init(uint32_t *w, uint32_t *h)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,
			GLFW_OPENGL_CORE_PROFILE);

	g_win = glfwCreateWindow(*w, *h, "be", NULL, NULL);
	CBUG(!g_win, "glfwCreateWindow");

	glfwMakeContextCurrent(g_win);
	glfwSwapInterval(1);

	glfwGetFramebufferSize(g_win, (int32_t *) w, (int32_t *) h);

#ifndef __APPLE__
	CBUG(glewInit() != GLEW_OK, "glfwCreateWindow");
#endif

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &VS_PRESENT, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &FS_PRESENT, NULL);
	glCompileShader(fs);

	g_prog_present = glCreateProgram();
	glAttachShader(g_prog_present, vs);
	glAttachShader(g_prog_present, fs);
	glLinkProgram(g_prog_present);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glGenVertexArrays(1, &g_vao_present);
	glBindVertexArray(g_vao_present);

	GLint loc = glGetUniformLocation(g_prog_present, "uTex");
	glUseProgram(g_prog_present);
	glUniform1i(loc, 0);

}

static void glfw_flush(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, qgl_width, qgl_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(g_prog_present);
	glBindVertexArray(g_vao_present);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_tex);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glfwSwapBuffers(g_win);
	glfwPollEvents();
}

static void glfw_deinit(void)
{
	glfwDestroyWindow(g_win);
	glfwTerminate();
}

void
qlfw_construct(void)
{
	qgl_glfw.init = glfw_init;
	qgl_glfw.deinit = glfw_deinit;
	qgl_glfw.flush = glfw_flush;
}
