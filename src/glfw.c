#include "./gl.h"
#include "./be.h"

#include <stdlib.h>

#include <GLFW/glfw3.h>

#include <ttypt/qsys.h>

qgl_be_t qgl_glfw;

GLFWwindow *g_win;

extern GLuint g_tex;

static void glfw_init(uint32_t *w, uint32_t *h)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	g_win = glfwCreateWindow(*w, *h, "be", NULL, NULL);
	CBUG(!g_win, "glfwCreateWindow");

	glfwMakeContextCurrent(g_win);
	glfwSwapInterval(1);

	glfwGetFramebufferSize(g_win, (int32_t *) w, (int32_t *) h);

#ifndef __APPLE__
	CBUG(glewInit() != GLEW_OK, "glfwCreateWindow");
#endif
}


static void glfw_flush(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, qgl_width, qgl_height);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, g_tex);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0,1); glVertex2f(0, 0);
	glTexCoord2f(1,1); glVertex2f(qgl_width, 0);
	glTexCoord2f(0,0); glVertex2f(0, qgl_height);
	glTexCoord2f(1,0); glVertex2f(qgl_width, qgl_height);
	glEnd();

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
