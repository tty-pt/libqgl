#ifndef QGL_GL_H
#define QGL_GL_H

#define GL_GLEXT_PROTOTYPES 1
#define GLEW_STATIC

#ifdef __APPLE__
# define GL_SILENCE_DEPRECATION
# include <OpenGL/glext.h>
# include <OpenGL/gl.h>
#else
# include <GL/glew.h>
# include <GL/gl.h>
#endif

#endif
