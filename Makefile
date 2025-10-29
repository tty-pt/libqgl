BE := glfw

INPUT-fb := dev
INPUT-glfw := glfw

LDLIBS-Linux += -lGL -lX11 -lGLEW -lglfw
LDLIBS-OpenBSD += -lGL -lGLU -lX11 -lGLEW -lglfw
LDLIBS-Windows += -lglew32 -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32
LDLIBS-Darwin += -lglfw

LDFLAGS-Darwin += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

add-prefix-OpenBSD += /usr/X11R6

LDLIBS := -lm -lxxhash -lqmap -lqsys -lpng
LDLIBS += ${LDLIBS-${BE}}

LDLIBS-Linux += -lEGL

obj-y := be glfw img png
obj-y += input input-glfw
# obj-y += input input-${INPUT-${BE}}
libqgl-obj-y := ${obj-y:%=src/%.o}
posix := fb input-dev
libqgl-obj-y-Linux := ${posix:%=src/%.o}
libqgl-obj-y-OpenBSD := ${posix:%=src/%.o}

CFLAGS := -g
all := libqgl

-include ../mk/include.mk
